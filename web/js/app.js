const rankingBody = document.getElementById('ranking-body');
const runButton = document.getElementById('run-search');
const datasetSize = document.getElementById('dataset-size');
const modeTabs = [...document.querySelectorAll('.mode-tab')];
const idInputWrap = document.getElementById('id-input-wrap');
const targetIdInput = document.getElementById('target-id');


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
  { id: 91823, name: "ShadowBlade", score: 9980, tier: "챌린저", width: 120 },
  { id: 445221, name: "NightFury", score: 9870, tier: "챌린저", width: 90 },
  { id: 103942, name: "CrimsonAce", score: 9750, tier: "챌린저", width: 110 },
  { id: 887341, name: "StarlightX", score: 9620, tier: "챌린저", width: 88 },
  { id: 234109, name: "IronWolf", score: 9540, tier: "챌린저", width: 84 },
  { id: 672019, name: "PixelKing", score: 9410, tier: "다이아", width: 92 },
  { id: 112984, name: "VoidHunter", score: 9300, tier: "다이아", width: 104 },
  { id: 509283, name: "FrostByte", score: 9180, tier: "다이아", width: 82 },
  { id: 384729, name: "ThunderX", score: 9050, tier: "다이아", width: 78 },
  { id: 721039, name: "NeonRift", score: 8920, tier: "다이아", width: 84 },
];

const presets = {
  single: {
    linearTime: '16.67 ms',
    linearOps: '1,000,000회',
    linearCaption: 'ID #500,000을 찾기 위해 테이블 절반 이상 순회',
    btreeTime: '11 μs',
    btreeOps: '500회',
    btreeCaption: '루트부터 분기하며 대상 키 탐색',
    bptreeTime: '3 μs',
    bptreeOps: '160회',
    bptreeCaption: '리프 노드까지 바로 이동해 조회 완료',
    lastSearch: 'ID #500,000',
    subtitle: '단일 탐색 결과 기준 샘플',
    bars: { linear: '100%', btree: '28%', bptree: '18%' },
  },
  range: {
    linearTime: '3.98 ms',
    linearOps: '1,001회',
    linearCaption: '범위 집계를 위해 매칭 레코드 전부 검사',
    btreeTime: '410 μs',
    btreeOps: '360회',
    btreeCaption: '중간 노드 분기 후 범위 확장',
    bptreeTime: '54 μs',
    bptreeOps: '88회',
    bptreeCaption: '연결된 리프 순회로 범위 탐색 최적',
    lastSearch: 'ID #250,000 ~ 251,000',
    subtitle: '범위 탐색 결과 기준 샘플',
    bars: { linear: '76%', btree: '32%', bptree: '12%' },
  },
  top10: {
    linearTime: '16.67 ms',
    linearOps: '1,000,000회',
    linearCaption: 'TOP 10 집계 완료 (전체 순회)',
    btreeTime: '11 μs',
    btreeOps: '500회',
    btreeCaption: '정렬 키 기준 탐색 후 상위 결과 수집',
    bptreeTime: '3 μs',
    bptreeOps: '160회',
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

// μs 단위 시간 포맷 (규약: ms 사용 금지, 0은 미측정)
function formatTimeUs(us) {
  if (!us || us <= 0) return '미측정';
  return formatNumber(us) + ' μs';
}

// sizes 배열에서 현재 선택된 데이터셋 크기의 인덱스 반환
function getSizeIndex(sizeValue) {
  if (!realData) return -1;
  return realData.sizes.indexOf(Number(sizeValue));
}

// 단일 탐색 모드일 때만 ID 입력 박스 표시
function updateIdInputVisibility() {
  idInputWrap.style.display = (currentMode === 'single') ? 'block' : 'none';
}

// 현재 입력된 ID 반환 (없으면 데이터셋 중간값 사용)
function getTargetId() {
  const val = parseInt(targetIdInput.value, 10);
  const size = Number(datasetSize.value);
  return (val > 0) ? val : Math.floor(size / 2);
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

  // 소요 시간 반영
  resultEls.linearTime.textContent  = formatTimeUs(linearUs);
  resultEls.btreeTime.textContent   = formatTimeUs(btreeUs);
  resultEls.bptreeTime.textContent  = formatTimeUs(bptreeUs);

  // 비교 횟수는 JSON 스키마에 없으므로 '미측정' 표시
  resultEls.linearOps.textContent  = '미측정';
  resultEls.btreeOps.textContent   = '미측정';
  resultEls.bptreeOps.textContent  = '미측정';

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
    summaryEls.lastSearch.textContent    = size + '건 범위 탐색';
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
              <span class="name-bar" style="width:${player.width}px"></span>
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
  summaryEls.count.textContent = formatNumber(Number(count));
  summaryEls.height.textContent = count === "100000" ? "3" : count === "500000" ? "4" : "4";
}

function applyPreset() {
  // 실측 데이터가 있으면 우선 적용 (single / range 모드)
  if (hasRealData) {
    const sizeIdx = getSizeIndex(datasetSize.value);
    if (applyRealData(currentMode, sizeIdx)) return;
  }

  // 실측 없거나 top10 모드 → preset 폴백
  const preset = presets[currentMode];

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

  // 단일 탐색 모드면 입력된 ID를 lastSearch에 반영
  if (currentMode === 'single') {
    summaryEls.lastSearch.textContent = 'ID #' + formatNumber(getTargetId());
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

datasetSize.addEventListener("change", () => {
  updateSummary(datasetSize.value);
});

runButton.addEventListener("click", () => {
  updateSummary(datasetSize.value);
  applyPreset();
  renderTable(topPlayers);
});

// 초기화: results.json 로드 후 UI 갱신
loadRealData().then(function () {
  updateIdInputVisibility();
  updateSummary(datasetSize.value);
  applyPreset();
  renderTable(topPlayers);
});
