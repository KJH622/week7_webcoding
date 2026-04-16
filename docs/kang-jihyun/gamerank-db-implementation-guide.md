# GameRank DB — 구현 방향 정리

> 팀 논의를 바탕으로 정리한 구현 방향 문서.
> 실사용 서비스 / 개념 정리는 별도 문서 참고.

---

## 목차

1. [프로젝트 컨셉](#1-프로젝트-컨셉)
2. [테이블 스키마](#2-테이블-스키마)
3. [구현할 자료구조 3가지](#3-구현할-자료구조-3가지)
4. [B+ 트리가 범위 검색에 유리한 이유 (구현 근거)](#4-b-트리가-범위-검색에-유리한-이유-구현-근거)
5. [실제 DBMS와 우리 구현의 차이](#5-실제-dbms와-우리-구현의-차이)
6. [왜 id만 인덱스를 타고 name/score는 선형 탐색인가](#6-왜-id만-인덱스를-타고-namescore는-선형-탐색인가)
7. [삽입이 많아도 인덱스를 유지해야 하는 이유](#7-삽입이-많아도-인덱스를-유지해야-하는-이유)
8. [B+ 트리 vs 해시 인덱스 vs LSM Tree](#8-b-트리-vs-해시-인덱스-vs-lsm-tree)
9. [SQL 처리기 연동 — WHERE 분기 로직](#9-sql-처리기-연동--where-분기-로직)
10. [벤치마크 설계](#10-벤치마크-설계)
11. [UI 구성 방향](#11-ui-구성-방향)

---

## 1. 프로젝트 컨셉

**GameRank DB** — 게임 플레이어 랭킹 DB

선정 이유:
- 100만 유저라는 규모가 현실적으로 납득됨
- `WHERE id = ?` 과제 요구사항과 1:1로 맞아떨어짐
- 범위 탐색 (`WHERE id BETWEEN ? AND ?`) 도 자연스럽게 설명 가능
- 발표 시 흥미를 끌기 좋음

---

## 2. 테이블 스키마

```c
typedef struct {
    int  id;        // Primary Key (자동 부여, 1부터 순차 증가)
    char name[32];  // 플레이어 닉네임
    int  score;     // 점수 (0 ~ 9999)
    char tier[16];  // 티어
} Player;
```

### 티어 기준

| 티어 | 점수 기준 |
|---|---|
| Challenger | 9500 이상 |
| Diamond | 8500 이상 |
| Platinum | 7000 이상 |
| Gold | 5000 이상 |
| Silver | 5000 미만 |

---

## 3. 구현할 자료구조 3가지

다른 조와의 차별점: **선형 탐색 / B 트리 / B+ 트리를 동시에 구현하고 비교**

| 구현 대상 | 파일 | 우선순위 |
|---|---|---|
| 선형 탐색 | `src/linear/linear_search.c` | 1순위 (가장 단순) |
| B+ 트리 | `src/bplus_tree/bplus_tree.c` | 2순위 (핵심) |
| B 트리 | `src/btree/btree.c` | 3순위 (여유 있을 때) |

> **전략**: 자료조사 조 기준 B+ 트리 개념 정리를 먼저 완료하고, 개발 조가 구현 시 참고할 수 있도록 핵심 로직과 인터페이스를 문서화한다.
> B 트리는 시간 여유가 있을 때 추가.

### 각 자료구조 구현 범위

**선형 탐색**
```c
Player *linear_search(Player *table, int size, int target_id);  // 단일
int     linear_range(Player *table, int size, int lo, int hi);  // 범위
```

**B+ 트리**
```c
void   bptree_insert(BPTree *tree, int key, void *record_ptr);
void  *bptree_search(BPTree *tree, int key);       // 단일 탐색
int    bptree_range(BPTree *tree, int lo, int hi); // 범위 탐색 (리프 연결 리스트 활용)
```

**B 트리**
```c
void   btree_insert(BTree *tree, int key, void *record_ptr);
void  *btree_search(BTree *tree, int key);
int    btree_range(BTree *tree, int lo, int hi);   // 트리 전체 순회 필요
```

---

## 4. B+ 트리가 범위 검색에 유리한 이유 (구현 근거)

B+ 트리에서 범위 탐색 구현 방식:

```
B 트리 범위 탐색 (BETWEEN 30 AND 60):
  루트 → 30 찾기 → 루트로 다시 올라감 → 40 찾기 → ... (매번 트리 재탐색)
  시간 복잡도: O(k × log n)

B+ 트리 범위 탐색:
  루트 → 리프(30) 찾기 → 옆 리프(40) → 옆 리프(50) → 옆 리프(60) → 종료
  시간 복잡도: O(log n + k)
```

`k`가 클수록 (범위가 넓을수록) B+ 트리의 이점이 극대화된다.

**리프 노드 연결 리스트 구현 포인트**:
```c
typedef struct BPLeaf {
    int            keys[BP_ORDER];
    void          *ptrs[BP_ORDER];
    int            num_keys;
    struct BPLeaf *next;   // ← 이 포인터가 범위 탐색의 핵심
} BPLeaf;
```

범위 탐색 구현 흐름:
1. `lo` 값으로 리프 노드 탐색
2. 해당 리프에서 `lo` 이상인 키부터 수집
3. `next` 포인터를 따라 이동하며 `hi` 이하인 키 계속 수집
4. `hi` 초과 키를 만나면 종료

---

## 5. 실제 DBMS와 우리 구현의 차이

구현 전 팀원 모두가 알아야 할 차이점:

| 항목 | 실제 MySQL InnoDB | 우리 과제 |
|---|---|---|
| 저장 위치 | 디스크 (16KB 페이지 단위) | RAM (포인터 기반) |
| 핵심 알고리즘 | **동일** | **동일** |
| 트리 높이 (100만 건) | 약 3~4 레벨 | 동일 |
| 영속성 | 서버 재시작 후에도 유지 | 프로세스 종료 시 사라짐 |
| 동시성 제어 | 트랜잭션, Lock, MVCC | 없음 (단일 스레드) |
| 성능 병목 | 디스크 I/O | 없음 (메모리 접근만) |

**결론**: 이번 과제는 핵심 알고리즘 구현이 목표이므로 실제 DBMS와 본질은 동일하다.

---

## 6. 왜 id만 인덱스를 타고 name/score는 선형 탐색인가

인덱스는 특정 컬럼에 만들어진 별도 자료구조이기 때문이다.

```c
// id 컬럼 → B+ 트리 인덱스 있음 → O(log n)
WHERE id = 500000     ✓ 인덱스 탐색

// name 컬럼 → 인덱스 없음 → O(n)
WHERE name = 'Alice'  ✗ 선형 탐색

// score 컬럼 → 인덱스 없음 → O(n)
WHERE score > 9000    ✗ 선형 탐색
```

모든 컬럼에 인덱스를 걸지 않는 이유:
- 인덱스마다 별도 B+ 트리가 생겨 **메모리 / 디스크 공간 증가**
- INSERT / UPDATE / DELETE 시 모든 인덱스를 갱신해야 해 **쓰기 속도 저하**

**이번 과제 적용**: `id` 컬럼에만 B+ 트리 인덱스를 적용하고,
`name`, `score` 조건은 선형 탐색으로 처리 → 이 차이가 벤치마크의 핵심 비교 지점이다.

---

## 7. 삽입이 많아도 인덱스를 유지해야 하는 이유

과제에서 100만 건을 INSERT할 때 B+ 트리 인덱스도 함께 유지해야 한다.

삽입 시 발생하는 비용:
- 리프 노드에 키 삽입
- 노드가 꽉 차면 **분할(Split)** → 부모 노드에 키 추가
- 최악의 경우 루트까지 분할이 전파됨 (그래도 O(log n))

그럼에도 유지하는 이유:
- 대부분의 서비스는 읽기 : 쓰기 = **10:1 ~ 100:1**
- 100만 건 삽입은 일회성이지만, 탐색은 수천 번 반복됨
- 삽입 시 O(log n) 비용은 탐색에서 돌려받는 이익이 압도적으로 큼

---

## 8. B+ 트리 vs 해시 인덱스 vs LSM Tree

우리가 B+ 트리를 선택한 이유를 설명할 수 있어야 한다.

| 구분 | B+ 트리 | 해시 인덱스 | LSM Tree |
|---|---|---|---|
| 단일 탐색 | O(log n) | **O(1)** | O(log n) |
| 범위 탐색 | **최강** | 불가 | 가능하지만 느림 |
| 정렬 (ORDER BY) | **최강** | 불가 | 가능 |
| 쓰기 성능 | 보통 | 보통 | **최강** |
| 대표 DB | MySQL, PostgreSQL | MySQL MEMORY 엔진 | Cassandra, RocksDB |

**우리 과제에서 B+ 트리를 선택한 이유**:
- 단일 탐색(`WHERE id = ?`)과 범위 탐색(`BETWEEN`) 모두 필요
- 랭킹 정렬(`ORDER BY score`) 에도 유리
- 해시 인덱스는 범위 탐색 불가, LSM Tree는 구현 복잡도가 너무 높음

---

## 9. SQL 처리기 연동 — WHERE 분기 로직

옵티마이저 없이 수동으로 분기하는 방법. 자료조사 조가 인터페이스를 정의하고 개발 조에 전달한다.

### 분기 흐름

```c
void execute_select(ParsedQuery *query, Table *table, BPTree *index) {

    if (query->where_col != NULL && strcmp(query->where_col, "id") == 0) {

        if (query->where_op == OP_EQ) {
            // WHERE id = ? → B+ 트리 단일 탐색
            void *record = bptree_search(index, atoi(query->where_val));
            print_record(record);

        } else if (query->where_op == OP_BETWEEN) {
            // WHERE id BETWEEN ? AND ? → B+ 트리 범위 탐색
            int cnt = bptree_range(index, query->range_lo, query->range_hi);
            printf("%d건 반환\n", cnt);

        } else if (query->where_op == OP_GT || query->where_op == OP_LT) {
            // WHERE id > ? / WHERE id < ? → B+ 트리 범위 탐색
            int cnt = bptree_range(index, query->range_lo, query->range_hi);
            printf("%d건 반환\n", cnt);
        }

    } else {
        // WHERE name = ? / WHERE score > ? / WHERE 절 없음 → 선형 탐색
        for (int i = 0; i < table->row_count; i++) {
            if (matches(&table->rows[i], query))
                print_record(&table->rows[i]);
        }
    }
}
```

### 분기 조건 요약

| SQL 조건 | 탐색 방법 | 시간 복잡도 |
|---|---|---|
| `WHERE id = 숫자` | B+ 트리 단일 탐색 | O(log n) |
| `WHERE id BETWEEN a AND b` | B+ 트리 범위 탐색 | O(log n + k) |
| `WHERE id > 숫자` | B+ 트리 범위 탐색 | O(log n + k) |
| `WHERE name = '...'` | 선형 탐색 | O(n) |
| `WHERE score > 숫자` | 선형 탐색 | O(n) |
| WHERE 절 없음 | 전체 선형 탐색 | O(n) |

### 개발 조와 맞춰야 할 인터페이스

```c
// 자료조사 조가 정의하고 개발 조에 전달할 함수 시그니처
BPTree *bptree_create(void);
void    bptree_insert(BPTree *tree, int key, void *record_ptr);
void   *bptree_search(BPTree *tree, int key);
int     bptree_range(BPTree *tree, int lo, int hi);
void    bptree_free(BPTree *tree);
```

---

## 10. 벤치마크 설계

### 측정 방법

```c
#include <sys/time.h>

long long now_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

long long t0 = now_us();
// 탐색 실행
long long t1 = now_us();
printf("소요 시간: %lld μs\n", t1 - t0);
```

### 측정 대상

| 탐색 종류 | 선형 탐색 | B 트리 | B+ 트리 |
|---|---|---|---|
| 단일 탐색 (`id = ?`) | O(n) | O(log n) | O(log n) |
| 범위 탐색 (`BETWEEN`) | O(n) | O(k × log n) | O(log n + k) |

### 레코드 수별 반복 측정

| 레코드 수 | 단일 탐색 | 범위 탐색 |
|---|---|---|
| 10만 | O | O |
| 50만 | O | O |
| 100만 | O | O |
| 500만 | O | O |

### 예상 결과

- 단일 탐색: 선형 >> B 트리 ≈ B+ 트리
- 범위 탐색: 선형 >> B 트리 > **B+ 트리 (압도적 우위)**
- 범위 탐색에서 B+ 트리의 우위를 데이터로 증명하는 것이 핵심

---

## 11. UI 구성 방향

### 레이아웃

```
┌──────────────────────────────────────────────┐
│  GameRank DB              B+ Tree Index Demo  │
│  [총 플레이어 수]  [트리 높이]  [마지막 탐색]    │ ← 요약 카드
├──────────────────────────────────────────────┤
│  Player ID 입력창             [레코드 수 선택]  │
│  [단일 탐색]  [범위 탐색]  [TOP 10 랭킹]        │ ← 탐색 유형 선택
│  [           탐색 실행           ]             │
├──────────────────────────────────────────────┤
│   선형 탐색    │    B 트리    │   B+ 트리 ★    │ ← 3개 카드
│  시간 / 횟수   │  시간 / 횟수  │  시간 / 횟수   │
│  [막대 게이지]  │ [막대 게이지] │ [막대 게이지]  │
├──────────────────────────────────────────────┤
│  랭킹 테이블 (검색한 플레이어 행 하이라이트)      │
└──────────────────────────────────────────────┘
```

### 발표 포인트

- 탐색 유형을 **범위 탐색**으로 전환하면 B+ 트리가 압도적으로 유리한 것이 막대 게이지로 시각적으로 보임
- 실제 C 구현에서 측정한 `gettimeofday()` 수치를 그대로 입력하면 발표용 데모로 사용 가능
- "우리 팀은 세 가지 방식을 직접 구현하고 데이터로 비교했다"는 설득력 확보

---

## 관련 문서

- [B+ 트리 개념 정리](bplus_tree_개념정리.md)
- [실사용 서비스 & 심화 개념 정리](btree_실사용_및_심화정리.md)
- [기획 및 설계 문서](planning.md)