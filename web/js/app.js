const rankingBody = document.getElementById("ranking-body");
const runButton = document.getElementById("run-search");
const datasetSize = document.getElementById("dataset-size");
const fastMode = document.getElementById("fast-mode");
const modeTabs = [...document.querySelectorAll(".mode-tab")];

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
    default: {
      linearTime: "16.67 ms",
      linearOps: "1,000,000회",
      linearCaption: "ID #500,000을 찾기 위해 테이블 절반 이상 순회",
      btreeTime: "11 μs",
      btreeOps: "500회",
      btreeCaption: "루트부터 분기하며 대상 키 탐색",
      bptreeTime: "3 μs",
      bptreeOps: "160회",
      bptreeCaption: "리프 노드까지 바로 이동해 조회 완료",
      lastSearch: "ID #500,000",
      subtitle: "단일 탐색 결과 기준 샘플",
      bars: { linear: "100%", btree: "28%", bptree: "18%" },
    },
    fast: {
      linearTime: "14.08 ms",
      linearOps: "500,000회",
      linearCaption: "캐시가 일부 유리해졌지만 여전히 전체 순회",
      btreeTime: "8 μs",
      btreeOps: "410회",
      btreeCaption: "탐색 경로 최적화 적용",
      bptreeTime: "2 μs",
      bptreeOps: "120회",
      bptreeCaption: "리프 포인터 접근으로 가장 빠르게 응답",
      lastSearch: "ID #432,484",
      subtitle: "빠른 모드 단일 탐색 샘플",
      bars: { linear: "90%", btree: "24%", bptree: "14%" },
    },
  },
  range: {
    default: {
      linearTime: "3.98 ms",
      linearOps: "1,001회",
      linearCaption: "범위 집계를 위해 매칭 레코드 전부 검사",
      btreeTime: "410 μs",
      btreeOps: "360회",
      btreeCaption: "중간 노드 분기 후 범위 확장",
      bptreeTime: "54 μs",
      bptreeOps: "88회",
      bptreeCaption: "연결된 리프 순회로 범위 탐색 최적",
      lastSearch: "ID #250,000 ~ 251,000",
      subtitle: "범위 탐색 결과 기준 샘플",
      bars: { linear: "76%", btree: "32%", bptree: "12%" },
    },
    fast: {
      linearTime: "3.21 ms",
      linearOps: "1,001회",
      linearCaption: "범위 일치 건수 집계 완료",
      btreeTime: "301 μs",
      btreeOps: "250회",
      btreeCaption: "범위 진입 지점 탐색 후 순차 검사",
      bptreeTime: "39 μs",
      bptreeOps: "64회",
      bptreeCaption: "리프 체인으로 빠르게 1001건 확인",
      lastSearch: "ID #780,100 ~ 781,100",
      subtitle: "빠른 모드 범위 탐색 샘플",
      bars: { linear: "68%", btree: "26%", bptree: "10%" },
    },
  },
  top10: {
    default: {
      linearTime: "16.67 ms",
      linearOps: "1,000,000회",
      linearCaption: "TOP 10 집계 완료 (전체 순회)",
      btreeTime: "11 μs",
      btreeOps: "500회",
      btreeCaption: "정렬 키 기준 탐색 후 상위 결과 수집",
      bptreeTime: "3 μs",
      bptreeOps: "160회",
      bptreeCaption: "TOP 10 집계 완료 (리프 순회만으로 처리)",
      lastSearch: "TOP 10 Ranking",
      subtitle: "상위 10명 샘플",
      bars: { linear: "100%", btree: "28%", bptree: "18%" },
    },
    fast: {
      linearTime: "15.13 ms",
      linearOps: "1,000,000회",
      linearCaption: "선형 정렬 기반 TOP 10 집계 완료",
      btreeTime: "9 μs",
      btreeOps: "420회",
      btreeCaption: "상위 키부터 빠르게 추출",
      bptreeTime: "2 μs",
      bptreeOps: "98회",
      bptreeCaption: "연속 리프 탐색으로 최상위 랭크 즉시 확보",
      lastSearch: "TOP 10 Ranking",
      subtitle: "빠른 모드 TOP 10 샘플",
      bars: { linear: "92%", btree: "24%", bptree: "12%" },
    },
  },
};

let currentMode = "single";

function formatNumber(value) {
  return new Intl.NumberFormat("ko-KR").format(value);
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
  const speed = fastMode.checked ? "fast" : "default";
  const preset = presets[currentMode][speed];

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

  summaryEls.lastSearch.textContent = preset.lastSearch;
  summaryEls.tableSubtitle.textContent = preset.subtitle;
}

modeTabs.forEach((button) => {
  button.addEventListener("click", () => {
    currentMode = button.dataset.mode;
    modeTabs.forEach((tab) => tab.classList.toggle("active", tab === button));
    applyPreset();
  });
});

datasetSize.addEventListener("change", () => {
  updateSummary(datasetSize.value);
});

fastMode.addEventListener("change", applyPreset);

runButton.addEventListener("click", () => {
  updateSummary(datasetSize.value);
  applyPreset();
  renderTable(topPlayers);
});

updateSummary(datasetSize.value);
applyPreset();
renderTable(topPlayers);
