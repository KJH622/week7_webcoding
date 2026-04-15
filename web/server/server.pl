use strict;
use warnings;
use IO::Socket::INET;
use File::Spec;
use FindBin qw($Bin);

# 우선순위: 커맨드라인 인수 > 환경변수 > 기본값
my $port = $ARGV[0] || $ENV{PORT}     || 8000;
my $root = $ARGV[1] || $ENV{WEB_ROOT} || File::Spec->catdir($Bin, '..');

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

while (my $client = $server->accept()) {
  $client->recv(my $request, 4096);
  my ($method, $path) = $request =~ m{^(GET)\s+([^\s]+)};

  if (!$method) {
    close $client;
    next;
  }

  $path =~ s/\?.*$//;
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
