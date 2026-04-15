use strict;
use warnings;
use IO::Socket::INET;
use File::Spec;
use FindBin qw($Bin);

# 우선순위: 커맨드라인 인수 > 환경변수 > 기본값
my $port = $ARGV[0] || $ENV{PORT}     || 8000;
my $root = $ARGV[1] || $ENV{WEB_ROOT} || File::Spec->catdir($Bin, '..');

# bin/search 바이너리 경로 (web/server/ 기준으로 두 단계 위 = 프로젝트 루트)
my $search_bin = File::Spec->catfile($Bin, '..', '..', 'bin', 'search');

my %types = (
  html => 'text/html; charset=UTF-8',
  css  => 'text/css; charset=UTF-8',
  js   => 'application/javascript; charset=UTF-8',
  json => 'application/json; charset=UTF-8',
  png  => 'image/png',
  jpg  => 'image/jpeg',
  jpeg => 'image/jpeg',
  svg  => 'image/svg+xml',
  ico  => 'image/x-icon',
);

my $server = IO::Socket::INET->new(
  LocalAddr => '0.0.0.0',
  LocalPort => $port,
  Proto     => 'tcp',
  Listen    => 5,
  Reuse     => 1,
) or die "server start failed on port $port: $!\n";

print "Serving $root at http://127.0.0.1:$port\n";
print "Search API: http://127.0.0.1:$port/search\n";

# 쿼리 파라미터 파싱
sub parse_query {
  my ($qs) = @_;
  my %q;
  for my $pair (split /&/, $qs // '') {
    my ($k, $v) = split /=/, $pair, 2;
    next unless defined $k && defined $v;
    $v =~ s/%([0-9A-Fa-f]{2})/chr(hex($1))/ge;
    $q{$k} = $v;
  }
  return %q;
}

# JSON 응답 전송
sub send_json {
  my ($client, $body) = @_;
  print $client
    "HTTP/1.1 200 OK\r\n" .
    "Content-Type: application/json; charset=UTF-8\r\n" .
    "Access-Control-Allow-Origin: *\r\n" .
    "Content-Length: " . length($body) . "\r\n" .
    "Connection: close\r\n\r\n" .
    $body;
}

while (my $client = $server->accept()) {
  $client->recv(my $request, 4096);
  my ($method, $full_path) = $request =~ m{^(GET)\s+([^\s]+)};

  if (!$method) {
    close $client;
    next;
  }

  # 경로와 쿼리 분리
  my ($path, $qs) = split /\?/, $full_path, 2;

  # ── /search 엔드포인트 ──────────────────────────────
  if ($path eq '/search') {
    my %q = parse_query($qs);

    my $mode = $q{mode} // 'single';
    my $size = $q{size} // '1000000';

    # 파라미터 검증 (숫자·알파벳만 허용, 인젝션 방지)
    unless ($mode =~ /^(single|range)$/ && $size =~ /^\d{1,8}$/) {
      send_json($client, '{"error":"invalid params"}');
      close $client;
      next;
    }

    # bin/search 바이너리 존재 확인
    unless (-f $search_bin && -x $search_bin) {
      send_json($client, '{"error":"search binary not found — make search 실행 필요"}');
      close $client;
      next;
    }

    my $cmd;
    if ($mode eq 'single') {
      my $target = $q{target} // int($size / 2);
      $target =~ s/[^\d]//g;
      $target ||= int($size / 2);
      $cmd = "$search_bin single $size $target";
    } else {
      my $lo = $q{lo} // '1';
      my $hi = $q{hi} // int($size * 0.01);
      $lo =~ s/[^\d]//g;
      $hi =~ s/[^\d]//g;
      $lo ||= 1;
      $hi ||= int($size * 0.01);
      $cmd = "$search_bin range $size $lo $hi";
    }

    # 실행 (최대 60초 타임아웃)
    my $json = '';
    eval {
      local $SIG{ALRM} = sub { die "timeout\n" };
      alarm(60);
      $json = qx{$cmd 2>/dev/null};
      alarm(0);
    };
    if ($@) {
      $json = '{"error":"timeout — 데이터셋이 너무 큽니다"}';
    }
    $json ||= '{"error":"search 실행 실패"}';

    send_json($client, $json);
    close $client;
    next;
  }

  # ── 정적 파일 서빙 ──────────────────────────────────
  $path = '/' if !$path || $path eq '';
  $path = '/index.html' if $path eq '/';
  $path =~ s/%20/ /g;
  $path =~ s#\.\.##g;

  my $file = File::Spec->catfile($root, $path =~ s{^/}{}r);

  if (-d $file) {
    $file = File::Spec->catfile($file, 'index.html');
  }

  if (-f $file) {
    open my $fh, '<', $file or do {
      print $client "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
      close $client;
      next;
    };
    binmode $fh;
    local $/;
    my $body = <$fh>;
    close $fh;

    my ($ext) = $file =~ /\.([^.]+)$/;
    my $type = $types{lc($ext // '')} // 'application/octet-stream';
    print $client "HTTP/1.1 200 OK\r\nContent-Type: $type\r\nContent-Length: " . length($body) . "\r\nConnection: close\r\n\r\n";
    print $client $body;
  } else {
    my $body = "404 Not Found\n";
    print $client "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=UTF-8\r\nContent-Length: " . length($body) . "\r\nConnection: close\r\n\r\n$body";
  }

  close $client;
}
