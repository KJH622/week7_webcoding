const rankingBody = document.getElementById('ranking-body');
const runButton = document.getElementById('run-search');
const datasetSize = document.getElementById('dataset-size');
const modeTabs = [...document.querySelectorAll('.mode-tab')];
const idInputWrap = document.getElementById('id-input-wrap');
const targetIdInput = document.getElementById('target-id');
const rangeInputWrap = document.getElementById('range-input-wrap');
const rangeLoInput = document.getElementById('range-lo');
const rangeHiInput = document.getElementById('range-hi');


// 실측 데이터 (results.json 로드 후 저장)
let realData = null;
let hasRealData = false;

const summaryEls = {
  count: document.getElementById("player-count"),
  height: document.getElementById("tree-height"),
  lastSearch: document.getElementById("last-search"),
  tableSubtitle: document.getElementById("table-subtitle"),
};

const resultEls = {
  linearTime: document.getElementById("linear-time"),
  linearOps: document.getElementById("linear-ops"),
  linearCaption: document.getElementById("linear-caption"),
  linearBar: document.getElementById("linear-bar"),
  btreeTime: document.getElementById("btree-time"),
  btreeOps: document.getElementById("btree-ops"),
  btreeCaption: document.getElementById("btree-caption"),
  btreeBar: document.getElementById("btree-bar"),
  bptreeTime: document.getElementById("bptree-time"),
  bptreeOps: document.getElementById("bptree-ops"),
  bptreeCaption: document.getElementById("bptree-caption"),
  bptreeBar: document.getElementById("bptree-bar"),
};

const topPlayers = [
  { id: 91823, name: "ShadowBlade", score: 9980, tier: "챌린저" },
  { id: 445221, name: "NightFury", score: 9870, tier: "챌린저" },
  { id: 103942, name: "CrimsonAce", score: 9750, tier: "챌린저" },
  { id: 887341, name: "StarlightX", score: 9620, tier: "챌린저" },
  { id: 234109, name: "IronWolf", score: 9540, tier: "챌린저" },
  { id: 672019, name: "PixelKing", score: 9410, tier: "다이아" },
  { id: 112984, name: "VoidHunter", score: 9300, tier: "다이아" },
  { id: 509283, name: "FrostByte", score: 9180, tier: "다이아" },
  { id: 384729, name: "ThunderX", score: 9050, tier: "다이아" },
  { id: 721039, name: "NeonRift", score: 8920, tier: "다이아" },
];

// 데이터셋 크기별 샘플 preset (실측값 없을 때 폴백용)
const presets = {
  single: {
    100000: {
      linearTime: '1,667 μs', linearOps: '100,000회',
      linearCaption: 'ID #50,000을 찾기 위해 테이블 절반 이상 순회',
      btreeTime: '9 μs',  btreeOps: '420회',
      btreeCaption: '루트부터 분기하며 대상 키 탐색',
      bptreeTime: '2 μs', bptreeOps: '130회',
      bptreeCaption: '리프 노드까지 바로 이동해 조회 완료',
      subtitle: '단일 탐색 결과 기준 샘플 (10만)',
      bars: { linear: '100%', btree: '27%', bptree: '12%' },
    },
    500000: {
      linearTime: '8,335 μs', linearOps: '500,000회',
      linearCaption: 'ID #250,000을 찾기 위해 테이블 절반 이상 순회',
      btreeTime: '10 μs', btreeOps: '470회',
      btreeCaption: '루트부터 분기하며 대상 키 탐색',
      bptreeTime: '3 μs', bptreeOps: '150회',
      bptreeCaption: '리프 노드까지 바로 이동해 조회 완료',
      subtitle: '단일 탐색 결과 기준 샘플 (50만)',
      bars: { linear: '100%', btree: '28%', bptree: '16%' },
    },
    1000000: {
      linearTime: '16,670 μs', linearOps: '1,000,000회',
      linearCaption: 'ID #500,000을 찾기 위해 테이블 절반 이상 순회',
      btreeTime: '11 μs', btreeOps: '500회',
      btreeCaption: '루트부터 분기하며 대상 키 탐색',
      bptreeTime: '3 μs', bptreeOps: '160회',
      bptreeCaption: '리프 노드까지 바로 이동해 조회 완료',
      subtitle: '단일 탐색 결과 기준 샘플 (100만)',
      bars: { linear: '100%', btree: '28%', bptree: '18%' },
    },
    5000000: {
      linearTime: '83,350 μs', linearOps: '5,000,000회',
      linearCaption: 'ID #2,500,000을 찾기 위해 테이블 절반 이상 순회',
      btreeTime: '13 μs', btreeOps: '560회',
      btreeCaption: '루트부터 분기하며 대상 키 탐색',
      bptreeTime: '4 μs', bptreeOps: '180회',
      bptreeCaption: '리프 노드까지 바로 이동해 조회 완료',
      subtitle: '단일 탐색 결과 기준 샘플 (500만)',
      bars: { linear: '100%', btree: '26%', bptree: '16%' },
    },
  },
  range: {
    100000: {
      linearTime: '398 μs',    linearOps: '1,001회',
      linearCaption: '범위 집계를 위해 매칭 레코드 전부 검사',
      btreeTime: '350 μs', btreeOps: '310회',
      btreeCaption: '중간 노드 분기 후 범위 확장',
      bptreeTime: '45 μs', bptreeOps: '72회',
      bptreeCaption: '연결된 리프 순회로 범위 탐색 최적',
      lastSearch: 'ID #1 ~ 1,000',
      subtitle: '범위 탐색 결과 기준 샘플 (10만)',
      bars: { linear: '100%', btree: '88%', bptree: '11%' },
    },
    500000: {
      linearTime: '1,990 μs',  linearOps: '5,001회',
      linearCaption: '범위 집계를 위해 매칭 레코드 전부 검사',
      btreeTime: '390 μs', btreeOps: '340회',
      btreeCaption: '중간 노드 분기 후 범위 확장',
      bptreeTime: '50 μs', bptreeOps: '80회',
      bptreeCaption: '연결된 리프 순회로 범위 탐색 최적',
      lastSearch: 'ID #1 ~ 5,000',
      subtitle: '범위 탐색 결과 기준 샘플 (50만)',
      bars: { linear: '100%', btree: '80%', bptree: '11%' },
    },
    1000000: {
      linearTime: '3,980 μs',  linearOps: '10,001회',
      linearCaption: '범위 집계를 위해 매칭 레코드 전부 검사',
      btreeTime: '410 μs', btreeOps: '360회',
      btreeCaption: '중간 노드 분기 후 범위 확장',
      bptreeTime: '54 μs', bptreeOps: '88회',
      bptreeCaption: '연결된 리프 순회로 범위 탐색 최적',
      lastSearch: 'ID #1 ~ 10,000',
      subtitle: '범위 탐색 결과 기준 샘플 (100만)',
      bars: { linear: '100%', btree: '76%', bptree: '13%' },
    },
    5000000: {
      linearTime: '19,900 μs', linearOps: '50,001회',
      linearCaption: '범위 집계를 위해 매칭 레코드 전부 검사',
      btreeTime: '450 μs', btreeOps: '400회',
      btreeCaption: '중간 노드 분기 후 범위 확장',
      bptreeTime: '65 μs', bptreeOps: '100회',
      bptreeCaption: '연결된 리프 순회로 범위 탐색 최적',
      lastSearch: 'ID #1 ~ 50,000',
      subtitle: '범위 탐색 결과 기준 샘플 (500만)',
      bars: { linear: '100%', btree: '74%', bptree: '13%' },
    },
  },
  top10: {
    linearTime: '16,670 μs', linearOps: '1,000,000회',
    linearCaption: 'TOP 10 집계 완료 (전체 순회)',
    btreeTime: '11 μs', btreeOps: '500회',
    btreeCaption: '정렬 키 기준 탐색 후 상위 결과 수집',
    bptreeTime: '3 μs', bptreeOps: '160회',
    bptreeCaption: 'TOP 10 집계 완료 (리프 순회만으로 처리)',
    lastSearch: 'TOP 10 Ranking',
    subtitle: '상위 10명 샘플',
    bars: { linear: '100%', btree: '28%', bptree: '18%' },
  },
};

let currentMode = 'single';

function formatNumber(value) {
  return new Intl.NumberFormat('ko-KR').format(value);
}

// μs 단위 시간 포맷 (규약: ms 사용 금지)
// realMode=true: 실측값 표시 중 → 0은 측정 불가(< 1 μs)로 표시
// realMode=false: 초기값 0 → 미측정 표시
function formatTimeUs(us, realMode) {
  if (us <= 0) return realMode ? '< 1 μs' : '미측정';
  return formatNumber(us) + ' μs';
}

// sizes 배열에서 현재 선택된 데이터셋 크기의 인덱스 반환
function getSizeIndex(sizeValue) {
  if (!realData) return -1;
  return realData.sizes.indexOf(Number(sizeValue));
}

// 모드에 따라 입력창 표시/숨김
function updateIdInputVisibility() {
  idInputWrap.style.display    = (currentMode === 'single') ? 'block' : 'none';
  rangeInputWrap.style.display = (currentMode === 'range')  ? 'flex'  : 'none';
}

// 현재 입력된 ID 반환 (없으면 데이터셋 중간값 사용)
function getTargetId() {
  const val = parseInt(targetIdInput.value, 10);
  const size = Number(datasetSize.value);
  return (val > 0) ? val : Math.floor(size / 2);
}

// 범위 탐색 lo/hi 반환 (없으면 규약 기본값: 1 ~ size*0.01)
function getRangeLo() {
  const val = parseInt(rangeLoInput.value, 10);
  return (val > 0) ? val : 1;
}

function getRangeHi() {
  const val = parseInt(rangeHiInput.value, 10);
  const size = Number(datasetSize.value);
  return (val > 0) ? val : Math.floor(size * 0.01);
}

// /search 엔드포인트로 실시간 탐색 요청
function fetchSearch() {
  const mode = currentMode;
  const size = datasetSize.value;
  let url = '/search?mode=' + mode + '&size=' + size;

  if (mode === 'single') {
    url += '&target=' + getTargetId();
  } else {
    url += '&lo=' + getRangeLo() + '&hi=' + getRangeHi();
  }

  return fetch(url)
    .then(function (res) {
      if (!res.ok) throw new Error('서버 오류: ' + res.status);
      return res.json();
    })
    .then(function (data) {
      if (data.error) throw new Error(data.error);
      return data;
    });
}

// 실시간 탐색 결과를 UI에 반영
function applySearchResult(data) {
  // 소요 시간
  resultEls.linearTime.textContent  = formatTimeUs(data.linear_time,  true);
  resultEls.btreeTime.textContent   = formatTimeUs(data.btree_time,   true);
  resultEls.bptreeTime.textContent  = formatTimeUs(data.bptree_time,  true);

  // 비교 횟수
  resultEls.linearOps.textContent  = data.linear_ops  > 0 ? formatNumber(data.linear_ops)  + '회' : '미측정';
  resultEls.btreeOps.textContent   = data.btree_ops   > 0 ? formatNumber(data.btree_ops)   + '회' : '미측정';
  resultEls.bptreeOps.textContent  = data.bptree_ops  > 0 ? formatNumber(data.bptree_ops)  + '회' : '미측정';

  // 진행 바
  const maxUs = Math.max(data.linear_time, data.btree_time, data.bptree_time, 1);
  resultEls.linearBar.style.width  = Math.round((data.linear_time  / maxUs) * 100) + '%';
  resultEls.btreeBar.style.width   = Math.round((data.btree_time   / maxUs) * 100) + '%';
  resultEls.bptreeBar.style.width  = Math.round((data.bptree_time  / maxUs) * 100) + '%';

  // 캡션 및 요약
  const n = formatNumber(data.size);
  if (data.mode === 'single') {
    const tid = formatNumber(data.target);
    summaryEls.lastSearch.textContent    = 'ID #' + tid;
    summaryEls.tableSubtitle.textContent = '실측값 기준 (' + n + '건)';
    resultEls.linearCaption.textContent  = 'ID #' + tid + ' 선형 탐색 실측값 (' + n + '건)';
    resultEls.btreeCaption.textContent   = 'ID #' + tid + ' B 트리 탐색 실측값 (' + n + '건)';
    resultEls.bptreeCaption.textContent  = 'ID #' + tid + ' B+ 트리 탐색 실측값 (' + n + '건)';
  } else {
    const lo = formatNumber(data.lo);
    const hi = formatNumber(data.hi);
    summaryEls.lastSearch.textContent    = 'ID #' + lo + ' ~ ' + hi;
    summaryEls.tableSubtitle.textContent = '실측값 기준 (' + n + '건)';
    resultEls.linearCaption.textContent  = lo + '~' + hi + ' 선형 탐색 실측값 (' + n + '건)';
    resultEls.btreeCaption.textContent   = lo + '~' + hi + ' B 트리 탐색 실측값 (' + n + '건)';
    resultEls.bptreeCaption.textContent  = lo + '~' + hi + ' B+ 트리 탐색 실측값 (' + n + '건)';
  }
}

// results.json 비동기 로드
function loadRealData() {
  return fetch('/assets/results.json')
    .then(function (res) {
      if (!res.ok) throw new Error('fetch 실패: ' + res.status);
      return res.json();
    })
    .then(function (data) {
      // sizes 배열과 single/range 키가 있어야 유효한 스키마
      if (data && Array.isArray(data.sizes) && data.single && data.range) {
        realData = data;
        hasRealData = true;
      }
    })
    .catch(function () {
      // 로드 실패 시 preset으로 폴백 (콘솔 경고만 출력)
      console.warn('results.json 로드 실패 — 샘플 데이터로 표시합니다.');
    });
}

// 실측값을 결과 카드에 반영 (single / range 모드 전용)
// top10 모드는 JSON 스키마에 없으므로 preset 사용
function applyRealData(mode, sizeIdx) {
  const modeMap = { single: 'single', range: 'range' };
  const key = modeMap[mode];
  if (!key || sizeIdx < 0) return false;

  const src = realData[key];
  const linearUs  = src.linear[sizeIdx];
  const btreeUs   = src.btree[sizeIdx];
  const bptreeUs  = src.bptree[sizeIdx];

  // 셋 다 미측정(0)이면 실측 데이터 미사용으로 간주
  if (!linearUs && !btreeUs && !bptreeUs) return false;

  // 소요 시간 반영 (realMode=true: 0μs는 '< 1 μs'로 표시)
  resultEls.linearTime.textContent  = formatTimeUs(linearUs,  true);
  resultEls.btreeTime.textContent   = formatTimeUs(btreeUs,   true);
  resultEls.bptreeTime.textContent  = formatTimeUs(bptreeUs,  true);

  // 비교 횟수 반영 (_ops 필드가 있으면 실측값, 없으면 미측정)
  const linearOps  = src.linear_ops  ? src.linear_ops[sizeIdx]  : 0;
  const btreeOps   = src.btree_ops   ? src.btree_ops[sizeIdx]   : 0;
  const bptreeOps  = src.bptree_ops  ? src.bptree_ops[sizeIdx]  : 0;
  resultEls.linearOps.textContent  = linearOps  > 0 ? formatNumber(linearOps)  + '회' : '미측정';
  resultEls.btreeOps.textContent   = btreeOps   > 0 ? formatNumber(btreeOps)   + '회' : '미측정';
  resultEls.bptreeOps.textContent  = bptreeOps  > 0 ? formatNumber(bptreeOps)  + '회' : '미측정';

  // 캡션 갱신
  const size = formatNumber(realData.sizes[sizeIdx]);
  resultEls.linearCaption.textContent  = size + '건 선형 탐색 실측값';
  resultEls.btreeCaption.textContent   = size + '건 B 트리 탐색 실측값';
  resultEls.bptreeCaption.textContent  = size + '건 B+ 트리 탐색 실측값';

  // 진행 바: 선형 기준 100%, 나머지 비례
  const maxUs = Math.max(linearUs, btreeUs, bptreeUs, 1);
  resultEls.linearBar.style.width  = Math.round((linearUs  / maxUs) * 100) + '%';
  resultEls.btreeBar.style.width   = Math.round((btreeUs   / maxUs) * 100) + '%';
  resultEls.bptreeBar.style.width  = Math.round((bptreeUs  / maxUs) * 100) + '%';

  // 마지막 탐색 / 서브타이틀 갱신
  if (mode === 'single') {
    const tid = formatNumber(getTargetId());
    summaryEls.lastSearch.textContent    = 'ID #' + tid;
    resultEls.linearCaption.textContent  = 'ID #' + tid + ' 선형 탐색 실측값 (' + size + '건)';
    resultEls.btreeCaption.textContent   = 'ID #' + tid + ' B 트리 탐색 실측값 (' + size + '건)';
    resultEls.bptreeCaption.textContent  = 'ID #' + tid + ' B+ 트리 탐색 실측값 (' + size + '건)';
  } else {
    const lo = formatNumber(getRangeLo());
    const hi = formatNumber(getRangeHi());
    summaryEls.lastSearch.textContent = 'ID #' + lo + ' ~ ' + hi;
  }
  summaryEls.tableSubtitle.textContent = '실측값 기준 (' + size + '건)';

  return true;
}

function renderTable(rows) {
  rankingBody.innerHTML = rows
    .map(
      (player, index) => `
        <tr>
          <td>${index + 1}</td>
          <td>#${formatNumber(player.id)}</td>
          <td>
            <div class="player-name">
              <span>${player.name}</span>
            </div>
          </td>
          <td>${formatNumber(player.score)}</td>
          <td><span class="tier-badge">${player.tier}</span></td>
        </tr>
      `
    )
    .join("");
}

function updateSummary(count) {
  const heightMap = { '100000': '3', '500000': '4', '1000000': '4', '5000000': '5' };
  summaryEls.count.textContent  = formatNumber(Number(count));
  summaryEls.height.textContent = heightMap[count] || '4';
}

function applyPreset() {
  // 실측 데이터가 있으면 우선 적용 (single / range 모드)
  if (hasRealData) {
    const sizeIdx = getSizeIndex(datasetSize.value);
    if (applyRealData(currentMode, sizeIdx)) return;
  }

  // 실측 없거나 top10 모드 → 크기별 preset 폴백
  const sizeKey = Number(datasetSize.value);
  const preset = (currentMode === 'top10')
    ? presets.top10
    : (presets[currentMode][sizeKey] || presets[currentMode][1000000]);

  resultEls.linearTime.textContent = preset.linearTime;
  resultEls.linearOps.textContent = preset.linearOps;
  resultEls.linearCaption.textContent = preset.linearCaption;
  resultEls.linearBar.style.width = preset.bars.linear;

  resultEls.btreeTime.textContent = preset.btreeTime;
  resultEls.btreeOps.textContent = preset.btreeOps;
  resultEls.btreeCaption.textContent = preset.btreeCaption;
  resultEls.btreeBar.style.width = preset.bars.btree;

  resultEls.bptreeTime.textContent = preset.bptreeTime;
  resultEls.bptreeOps.textContent = preset.bptreeOps;
  resultEls.bptreeCaption.textContent = preset.bptreeCaption;
  resultEls.bptreeBar.style.width = preset.bars.bptree;

  // 모드별 lastSearch 반영
  if (currentMode === 'single') {
    summaryEls.lastSearch.textContent = 'ID #' + formatNumber(getTargetId());
  } else if (currentMode === 'range') {
    summaryEls.lastSearch.textContent = 'ID #' + formatNumber(getRangeLo()) + ' ~ ' + formatNumber(getRangeHi());
  } else {
    summaryEls.lastSearch.textContent = preset.lastSearch;
  }
  summaryEls.tableSubtitle.textContent = preset.subtitle;
}

modeTabs.forEach(function (button) {
  button.addEventListener('click', function () {
    currentMode = button.dataset.mode;
    modeTabs.forEach(function (tab) { tab.classList.toggle('active', tab === button); });
    updateIdInputVisibility();
    applyPreset();
  });
});

datasetSize.addEventListener('change', function () {
  updateSummary(datasetSize.value);
  applyPreset();
});

runButton.addEventListener('click', function () {
  updateSummary(datasetSize.value);
  renderTable(topPlayers);

  // top10 모드는 실시간 탐색 없음 → preset
  if (currentMode === 'top10') {
    applyPreset();
    return;
  }

  // 로딩 상태
  runButton.textContent = '탐색 중...';
  runButton.disabled = true;
  runButton.classList.add('loading');

  fetchSearch()
    .then(function (data) {
      applySearchResult(data);
    })
    .catch(function (err) {
      console.warn('실시간 탐색 실패, 사전 측정값 사용:', err.message);
      applyPreset();
    })
    .then(function () {
      // finally 대신 두 번째 .then으로 항상 실행
      runButton.textContent = '탐색 실행';
      runButton.disabled = false;
      runButton.classList.remove('loading');
    });
});

// 초기화: results.json 로드 후 UI 갱신
loadRealData().then(function () {
  updateIdInputVisibility();
  updateSummary(datasetSize.value);
  applyPreset();
  renderTable(topPlayers);
});
