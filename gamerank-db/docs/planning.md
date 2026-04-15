# 기획 및 설계 문서

## 컨셉

**GameRank DB** — 게임 플레이어 랭킹 DB를 컨셉으로 한 B+ 트리 인덱스 구현 프로젝트.
100만 명 이상의 플레이어 데이터를 대상으로 세 가지 탐색 방식의 성능을 비교한다.

---

## 결정 사항

### 차수 (Order)
- B 트리 / B+ 트리 공통: `ORDER = 4` (노드당 최대 4개 키)
- 필요 시 `#define BP_ORDER` 값을 조정해 성능 변화 관찰 가능

### 노드 구조
- 내부 노드 + 리프 노드 **통합 구조체** (`is_leaf` 플래그로 구분)
- 구현 단순화 우선

### 인터페이스 함수 (개발 조와 합의)
```c
void   bptree_insert(BPTree *tree, int key, void *record_ptr);
void  *bptree_search(BPTree *tree, int key);
int    bptree_range(BPTree *tree, int lo, int hi);
```

---

## 테이블 스키마

```c
typedef struct {
    int  id;        // Primary Key (자동 부여)
    char name[32];  // 플레이어 닉네임
    int  score;     // 점수 (0 ~ 9999)
    char tier[16];  // 티어 (Challenger / Diamond / Platinum / Gold / Silver)
} Player;
```

---

## 벤치마크 계획

| 레코드 수 | 단일 탐색 | 범위 탐색 |
|---|---|---|
| 10만 | O | O |
| 50만 | O | O |
| 100만 | O | O |
| 500만 | O | O |

### 측정 방법
- `gettimeofday()` 사용 (마이크로초 단위)
- 각 케이스 5회 측정 후 평균

---

## 차별점

1. 선형 탐색 / B 트리 / B+ 트리 **3방향 비교**
2. **범위 탐색** 비교 (B+ 트리 리프 연결 리스트 강점 부각)
3. 벤치마크 결과 **시각화 데모** (demo/index.html)
