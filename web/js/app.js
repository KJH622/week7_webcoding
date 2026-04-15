const rankingBody = document.getElementById("ranking-body");
const runButton = document.getElementById("run-search");
const datasetSize = document.getElementById("dataset-size");
const modeTabs = [...document.querySelectorAll(".mode-tab")];
const idInputWrap = document.getElementById("id-input-wrap");
const targetIdInput = document.getElementById("target-id");

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

const apiBase = new URL("./", window.location.href);

let currentMode = "single";
let benchmarkData = null;

function buildUrl(path) {
  return new URL(path, apiBase).toString();
}

function formatNumber(value) {
  return new Intl.NumberFormat("ko-KR").format(value);
}

function formatTimeUs(us) {
  if (typeof us !== "number") {
    return "미측정";
  }
  return `${us.toFixed(3)} μs`;
}

function estimateTreeHeight(count) {
  if (count <= 100000) {
    return "3";
  }
  if (count <= 1000000) {
    return "4";
  }
  return "5";
}

function updateSummary(count) {
  summaryEls.count.textContent = formatNumber(Number(count));
  summaryEls.height.textContent = estimateTreeHeight(Number(count));
}

function updateIdInputVisibility() {
  idInputWrap.style.display = currentMode === "single" ? "block" : "none";
}

function getTargetId() {
  const value = Number(targetIdInput.value);
  const count = Number(datasetSize.value);
  return Number.isInteger(value) && value > 0 ? value : Math.floor(count / 2);
}

async function fetchJson(url, label) {
  const response = await fetch(url, { cache: "no-store" });
  const text = await response.text();

  if (!response.ok) {
    let payload = null;

    try {
      payload = JSON.parse(text);
    } catch (error) {
      payload = null;
    }

    throw new Error(
      payload && payload.message
        ? `${label}: ${payload.message}`
        : `${label}: ${response.status} ${response.statusText}`
    );
  }

  try {
    return JSON.parse(text);
  } catch (error) {
    throw new Error(`${label}: ${text}`);
  }
}

async function ensureDataset(count) {
  if (benchmarkData && benchmarkData.meta && Number(benchmarkData.meta.dataset_size) === Number(count)) {
    return benchmarkData;
  }

  runButton.textContent = "CSV 생성 및 벤치마크 실행 중...";

  const generated = await fetchJson(buildUrl(`api/generate?count=${count}`), "데이터 생성 실패");
  if (!generated.ok) {
    throw new Error(generated.message || "데이터 생성 실패");
  }

  benchmarkData = await fetchJson(
    buildUrl(`assets/results.json?ts=${Date.now()}`),
    "results.json 로드 실패"
  );
  return benchmarkData;
}

function setProgressBars(values) {
  const maxValue = Math.max(values.linear, values.btree, values.bptree, 0.001);
  resultEls.linearBar.style.width = `${Math.max(10, Math.round((values.linear / maxValue) * 100))}%`;
  resultEls.btreeBar.style.width = `${Math.max(10, Math.round((values.btree / maxValue) * 100))}%`;
  resultEls.bptreeBar.style.width = `${Math.max(10, Math.round((values.bptree / maxValue) * 100))}%`;
}

function renderRows(rows) {
  rankingBody.innerHTML = rows.map((player, index) => `
    <tr>
      <td>${index + 1}</td>
      <td>#${formatNumber(player.id)}</td>
      <td>
        <div class="player-name">
          <span>${player.name}</span>
          <span class="name-bar" style="width:${player.width || 96}px"></span>
        </div>
      </td>
      <td>${formatNumber(player.score)}</td>
      <td><span class="tier-badge">${player.tier}</span></td>
    </tr>
  `).join("");
}

function renderTopPlayers(players) {
  const rows = (players || []).map((player, index) => ({
    ...player,
    width: 76 + ((10 - index) * 6),
  }));

  renderRows(rows);
  summaryEls.tableSubtitle.textContent = "상위 10명 샘플";
  summaryEls.lastSearch.textContent = "TOP 10 Ranking";
}

function renderRange(benchmark) {
  const range = benchmark.range_search;
  const count = benchmark.meta.dataset_size;

  resultEls.linearTime.textContent = formatTimeUs(range.linear.avg_us);
  resultEls.btreeTime.textContent = formatTimeUs(range.btree.avg_us);
  resultEls.bptreeTime.textContent = formatTimeUs(range.bptree.avg_us);

  resultEls.linearOps.textContent = `${formatNumber(range.count)}건`;
  resultEls.btreeOps.textContent = `${estimateTreeHeight(count)}단계 + 범위`;
  resultEls.bptreeOps.textContent = `${formatNumber(range.count)}개 리프`;

  resultEls.linearCaption.textContent = `${formatNumber(range.lo)}~${formatNumber(range.hi)} 범위를 선형 탐색으로 확인`;
  resultEls.btreeCaption.textContent = `${formatNumber(range.lo)}~${formatNumber(range.hi)} 범위를 B 트리로 분기`;
  resultEls.bptreeCaption.textContent = `${formatNumber(range.lo)}~${formatNumber(range.hi)} 범위를 연결된 리프로 순회`;

  setProgressBars({
    linear: range.linear.avg_us,
    btree: range.btree.avg_us,
    bptree: range.bptree.avg_us,
  });

  summaryEls.lastSearch.textContent = `ID #${formatNumber(range.lo)} ~ ${formatNumber(range.hi)}`;
  summaryEls.tableSubtitle.textContent = "범위 탐색 기준 상위 10명 샘플";
  renderTopPlayers(benchmark.top_players);
}

function renderSingle(payload, benchmark) {
  const count = benchmark.meta.dataset_size;
  const targetId = payload.target_id;

  resultEls.linearTime.textContent = formatTimeUs(payload.timings.linear_us);
  resultEls.btreeTime.textContent = formatTimeUs(payload.timings.btree_us);
  resultEls.bptreeTime.textContent = formatTimeUs(payload.timings.bptree_us);

  resultEls.linearOps.textContent = `${formatNumber(count)}회`;
  resultEls.btreeOps.textContent = `${estimateTreeHeight(count)}단계`;
  resultEls.bptreeOps.textContent = `${estimateTreeHeight(count)}단계`;

  resultEls.linearCaption.textContent = `ID #${formatNumber(targetId)}를 찾기 위해 선형 탐색 수행`;
  resultEls.btreeCaption.textContent = `ID #${formatNumber(targetId)}를 B 트리 분기로 탐색`;
  resultEls.bptreeCaption.textContent = `ID #${formatNumber(targetId)}를 B+ 트리 리프에서 탐색`;

  setProgressBars({
    linear: payload.timings.linear_us,
    btree: payload.timings.btree_us,
    bptree: payload.timings.bptree_us,
  });

  summaryEls.lastSearch.textContent = `ID #${formatNumber(targetId)}`;

  if (payload.player) {
    summaryEls.tableSubtitle.textContent = "검색 결과 1건";
    renderRows([{
      ...payload.player,
      width: 120,
    }]);
  } else {
    summaryEls.tableSubtitle.textContent = "검색 결과 없음";
    rankingBody.innerHTML = `
      <tr>
        <td colspan="5">ID #${formatNumber(targetId)} 에 해당하는 플레이어가 없습니다.</td>
      </tr>
    `;
  }
}

async function runCurrentMode() {
  const count = Number(datasetSize.value);
  updateSummary(count);

  try {
    const benchmark = await ensureDataset(count);

    if (currentMode === "single") {
      const targetId = getTargetId();
      runButton.textContent = "탐색 실행 중...";
      const payload = await fetchJson(buildUrl(`api/search?id=${targetId}`), "검색 실패");
      if (!payload.ok) {
        throw new Error(payload.message || "검색 실패");
      }
      renderSingle(payload, benchmark);
    } else if (currentMode === "range") {
      renderRange(benchmark);
    } else {
      resultEls.linearTime.textContent = formatTimeUs(benchmark.single_search.linear.avg_us);
      resultEls.btreeTime.textContent = formatTimeUs(benchmark.single_search.btree.avg_us);
      resultEls.bptreeTime.textContent = formatTimeUs(benchmark.single_search.bptree.avg_us);

      resultEls.linearOps.textContent = `${formatNumber(benchmark.meta.dataset_size)}회`;
      resultEls.btreeOps.textContent = `${estimateTreeHeight(benchmark.meta.dataset_size)}단계`;
      resultEls.bptreeOps.textContent = `${estimateTreeHeight(benchmark.meta.dataset_size)}단계`;

      resultEls.linearCaption.textContent = "TOP 10 집계를 위해 전체 데이터를 순회";
      resultEls.btreeCaption.textContent = "정렬된 구조를 바탕으로 상위권 정보를 확인";
      resultEls.bptreeCaption.textContent = "리프 구간을 활용해 상위권 정보를 빠르게 확인";

      setProgressBars({
        linear: benchmark.single_search.linear.avg_us,
        btree: benchmark.single_search.btree.avg_us,
        bptree: benchmark.single_search.bptree.avg_us,
      });

      renderTopPlayers(benchmark.top_players);
    }
  } catch (error) {
    window.alert(error.message || "실행 중 오류가 발생했습니다.");
  } finally {
    runButton.textContent = "탐색 실행";
  }
}

modeTabs.forEach((button) => {
  button.addEventListener("click", () => {
    currentMode = button.dataset.mode;
    modeTabs.forEach((tab) => tab.classList.toggle("active", tab === button));
    updateIdInputVisibility();
  });
});

datasetSize.addEventListener("change", () => {
  updateSummary(datasetSize.value);
});

runButton.addEventListener("click", runCurrentMode);
targetIdInput.addEventListener("keydown", (event) => {
  if (event.key === "Enter" && currentMode === "single") {
    runCurrentMode();
  }
});

updateIdInputVisibility();
updateSummary(datasetSize.value);
renderRows([
  { id: 91823, name: "ShadowBlade", score: 9980, tier: "챌린저", width: 120 },
  { id: 445221, name: "NightFury", score: 9870, tier: "챌린저", width: 110 },
  { id: 103942, name: "CrimsonAce", score: 9750, tier: "챌린저", width: 100 },
]);
