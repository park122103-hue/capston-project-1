# Time-Decay Initial Mapping 및 DAG Routing 기말 연구 로드맵

본 연구는 상호작용 가중치 $W(\alpha_M)$의 독립적인 영향, Router의 SWAP 비용에 적용되는 시간 가중치 $\alpha_R$의 영향, 그리고 두 요소를 결합했을 때의 효과를 측정한다. 모든 실험 방법에는 동일하게 정규화된 QASM 회로와 동일한 연결 그래프를 입력한다. 출력 회로는 동일한 분해 규칙에 따라 basis `{rz, sx, x, cx}`로 변환한다.

## 1. 팀원 ZIP 파일의 기존 코드 재사용

왼쪽 열의 경로는 모두 `qubit_project_theo_luong/`을 기준으로 한다.

| 팀원 ZIP 파일 | 연구 패키지의 파일 | 역할 |
|---|---|---|
| `01_luong_chinh/01_doc_mach_QASM/*.qasm` | `benchmarks/*.qasm` | 34개 회로의 원본 바이트를 그대로 유지한다. 파라미터 선택용 데이터셋과 평가용 데이터셋은 `config/cases.json`에 정의한다. |
| `01_luong_chinh/02_trong_so_va_khoang_cach/Graph.h` | `src/Graph.h` | 각 물리 큐비트에서 BFS를 수행하여 최단 경로 거리 행렬 $D_H$를 생성한다. |
| `01_luong_chinh/02_trong_so_va_khoang_cach/Circuit.h` | `src/Circuit.h` | 0부터 시작하는 CX 의존성 레이어를 재사용한다. $0.8^\ell$을 적용하고 레이어 0–59만 계산하는 기존 `member` 모드도 유지한다. |
| `01_luong_chinh/03_anh_xa_ban_dau/Mapper.cpp`, `Mapper.h` | `src/Mapper.cpp`, `src/Mapper.h` | Importance, Affinity, Centrality 기준 및 동점 처리 규칙을 포함하여 기존 Greedy Mapper의 동작을 그대로 유지한다. |
| `01_luong_chinh/04_dinh_tuyen_DAG/Router.cpp`, `Router.h` | `src/Router.cpp`, `src/Router.h` | DAG 라우팅의 핵심 로직, look-ahead window, SWAP 및 BRIDGE 선택을 유지한다. 여기에 $\alpha_R$ 파라미터, 실행 추적(trace) 모드 및 진행 정체 감지 기능을 추가한다. |
| `01_luong_chinh/05_dieu_phoi_va_bien_dich/main.cpp` | `src/member_main.cpp` | 팀원 버전의 다중 패스 실행 진입점이다. 간단한 재현 실험과 실제 SWAP 및 BRIDGE의 차이 확인에 사용한다. |
| `01_luong_chinh/06_do_luong_so_sanh/run_evaluation.py`, `run_secondary_topologies.py` | `config/topologies.json`, `evaluate.py`, `pipeline.py` | Tokyo20의 무방향 간선 35개, Line5의 간선 4개, Grid6의 간선 7개를 재사용한다. FakeQuitoV2 스냅샷에서 Quito5 그래프를 추가한다. 공통 측정 체계를 구성하고 Ubuntu/WSL에서 실행할 수 있도록 한다. |

대조를 위해 확인해야 할 파일은 `01_luong_chinh/07_kiem_tra_hanh_vi/audit_real_swaps.py`와 `02_khao_sat_tham_so/02_quet_alpha_beta/benchmark_grid_eval.cpp`이다. 첫 번째 파일은 BRIDGE를 나타내는 `(-1, gate_id)` 마커를 확인하는 데 사용된다. 두 번째 파일은 라우터의 탐색 창에서 alpha를 매개변수화하는 한 가지 방식을 보여 주며, 이를 메인 Router에 통합할 때 참고할 수 있다. Markov, oscillator, cavity, waypoint, ADAC 관련 분기는 이번 실험 범위에서 제외한다.

기존 ZIP의 주요 CSV 파일 세 개에는 각각 데이터 행이 20개씩 있다. 기존 표는 라우팅 동작 횟수를 `swaps_count`로 기록하고, 동작 횟수로부터 CX 수를 환산하였다. 또한 내부 C++ 실행 시간과 더 넓은 범위를 포함하는 Qiskit 실행 시간을 나란히 제시하였다. 이러한 기존 표는 재현 기준값을 확인하는 용도로만 사용한다. 대조 실험의 결론은 이번 패키지에서 새로 생성한 `results/` 자료를 기반으로 작성한다.

Quito5 관련 원본 자료와 SABRE의 두 모드에 관한 API 문서는 [FakeQuitoV2 설정](https://github.com/Qiskit/qiskit-ibm-runtime/blob/main/qiskit_ibm_runtime/fake_provider/backends/quito/conf_quito.json), [SabreLayout](https://quantum.cloud.ibm.com/docs/en/api/qiskit/2.5/qiskit.transpiler.passes.SabreLayout), [SabreSwap](https://quantum.cloud.ibm.com/docs/en/api/qiskit/2.5/qiskit.transpiler.passes.SabreSwap)을 참조한다.

## 2. $W(\alpha_M)$ 및 $\alpha_R$가 적용된 거리 비용의 정확한 정의

### Initial Mapping(초기 큐비트 매핑)

공통 basis로 변환한 논리 회로에서 $g=(a_g,b_g)$를 하나의 CX 게이트라고 하자. 각 게이트는 $W_{a_g b_g}$와 $W_{b_g a_g}$에 모두 $w_M(g)$만큼 기여한다. Greedy의 큐비트 배치 규칙에 사용하는 비용 함수는 다음과 같다.

$$
C(M)=\sum_{i<j} W_{ij}D_H(M(i),M(j)).
$$

| 가중치 프로파일 | CX 게이트의 가중치 | 실험 목적 |
|---|---|---|
| `uniform` | $1$ | 전체 상호작용 빈도를 사용하는 Greedy 기준선(baseline)이다. |
| `layer` | $\alpha_M^{\ell_g}$ | 앞부분의 CX 의존성 레이어를 우선시한다. 두 논리 큐비트 라인에서 모두 첫 번째인 CX 게이트는 $\ell_g=0$이다. |
| `normalized-layer` | $\alpha_M^{\ell_g/\max(1,\ell_{\max})}$ | 동일한 레이어 정의를 유지하면서 CX 깊이에 의한 정규화의 영향을 직접 확인한다. |
| `index` | $\alpha_M^{t_g}$ | 앞서 실행된 CX 게이트 수에 따라 앞부분의 게이트를 우선시한다. |
| `normalized-index` | $\alpha_M^{t_g/\max(1,G-1)}$ | CX 순서를 사용하는 방식으로 앞서 논의한 수식이다. |
| `member` | $\ell_g<60$이면 $0.8^{\ell_g}$, 그 외에는 $0$ | 팀원 ZIP의 고정 설정을 재현한다. |

여기서 $G$는 basis 변환 후 CX 게이트의 총수이고, $t_g$는 0부터 $G-1$까지의 인덱스이다. 게이트의 순서는 정규화된 Qiskit 회로에서 가져온다. 서로 독립적인 게이트는 정규화 과정에서 순서가 달라질 수 있다. 서로 겹치지 않는(disjoint) 큐비트 쌍의 CX 두 개는 $t_g$가 달라도 같은 의존성 레이어에 속할 수 있다. 동일한 위치 정의 내에서 raw 방식과 normalized 방식을 비교하면 정규화의 효과를 레이어 또는 인덱스 선택의 효과와 분리할 수 있다. 가중치는 기존 `Mapper.cpp`를 재사용하기 위해 $10^{12}$ 스케일의 고정소수점(fixed-point) 값으로 저장한다. raw 프로파일에서는 매우 작은 가중치가 반올림되어 0이 될 수 있다. CX 인덱스 기반 프로파일에서 alpha가 0.8이면 인덱스 127부터 각 CX의 가중치는 이 패키지의 수치 정밀도에서 0이 된다. `member` 프로파일에는 고정된 레이어 절단 기준도 추가로 존재한다.

### Routing(라우팅)

현재 탐색 창에서 $\ell_k(g)$는 게이트 $g$가 사용하는 두 큐비트 라인의 남은 의존성 큐에서 가장 멀리 떨어진 위치를 뜻한다. front layer의 게이트는 $\ell_k(g)=0$이다. SWAP 후보 $S$에 대해 Router는 다음과 같은 비용 변화를 평가한다.

$$
\Delta C_k(S)=
\sum_{g\in\mathcal W_k}
w_R(k,g)\,[D_H(M^S(a_g),M^S(b_g))-D_H(M(a_g),M(b_g))].
$$

기존 Router에는 slack이 있는 게이트에 대한 지연 관련 점수 감산, 되돌아가는 SWAP에 대한 페널티, BRIDGE 임계값, 다단계 후보 탐색도 포함되어 있다. 연구 패키지는 이러한 규칙과 지연 계수 0.8을 유지하면서, look-ahead 위치에 해당하는 $w_R(k,g)$의 계수를 $\alpha_R^{\ell_k(g)}$로 변경한다. $\alpha_R=0.8$이면 Router는 기존 가중치 테이블을 정확하게 재사용한다. $\alpha_R=1$이면 look-ahead 위치에 따른 계수가 모두 1이 된다. 주요 실험은 `member_main.cpp`의 pass 0에 해당하는 $b=0$ 조건에서 한 번의 순방향 패스로 수행한다.

이 항목은 *시간 가중 거리 비용(time-weighted distance cost)*이라고 부를 수 있다. $D_H$ 자체는 여전히 하드웨어 최단 경로의 간선 수이다. $\alpha_M$은 초기 레이아웃을 만드는 방식을 결정하고, $\alpha_R$은 현재 레이아웃에서 SWAP 동작을 평가하는 방식을 결정한다. 두 계수는 독립적으로 선택한다.

## 3. 대조 실험 구성

$W^*$와 $\alpha_R^*$는 `results/fit/selected.json`에서 가져온다.

| 실험 코드 | Initial Mapping | Router | 주요 비교 목적 |
|---|---|---|---|
| E1 | Greedy Uniform | 지정된 레이아웃을 사용하는 SABRE | W 효과를 평가하기 위한 주요 기준선이다. |
| E2 | Greedy $W^*$ | 지정된 레이아웃을 사용하는 SABRE | E2 − E1은 SABRE를 고정한 상태에서 W의 영향을 측정한다. |
| E3 | Greedy Uniform | 팀 Router, $\alpha_R=1$ | E3 − E1은 동일한 레이아웃에서 두 Router의 차이를 측정한다. |
| E4 | Greedy $W^*$ | 팀 Router, $\alpha_R=1$ | E4 − E3은 팀 Router에서 W의 영향을 측정한다. |
| E5 | Greedy Uniform | 팀 Router, $\alpha_R=\alpha_R^*$ | E5 − E3은 SWAP 비용의 시간 가중치가 미치는 영향을 측정한다. |
| E6 | Greedy $W^*$ | 팀 Router, $\alpha_R=\alpha_R^*$ | E6 − E4는 $\alpha_R$ 추가 효과를, E6 − E5는 W 추가 효과를 측정한다. |
| E7 | SABRE가 레이아웃을 자체 선택 | 통합 SABRE | E6 − E7은 동일한 컴파일 범위에서 두 전체 파이프라인을 비교한다. |
| E8 | SABRE의 레이아웃 선택을 별도 실행 | 지정된 레이아웃을 사용하는 SABRE | `map_wall_ms`를 E1/E2와 비교한다. SABRE의 레이아웃 탐색 시간에는 레이아웃을 찾기 위해 수행하는 내부 라우팅도 포함된다. |

`Member_1pass`는 레이어 59 이후 W를 절단하는 방식, $\alpha_M=\alpha_R=0.8$, Router의 단일 패스를 재현한다. holdout 결과의 `W_...`로 시작하는 네 개 실험 항목은 fit 데이터셋에서 각 W 프로파일별로 선택된 최적 계수를 사용하며, 모두 고정 레이아웃을 입력받는 SABRE를 사용한다.

두 개선 요소 간의 상호작용을 확인하려면 `paired.csv`의 `interaction_W_x_alphaR`를 확인한다. 값이 작을수록 좋은 평가지표에서 $E6-E5-E4+E3<0$이면, 해당 사례의 결합 효과가 두 개별 효과의 합보다 더 좋다는 뜻이다. 다만 E6 − E4와 E6 − E5도 함께 확인하여 결합 방식이 각 개별 방식보다 실제로 지표를 줄이는지 판단해야 한다.

## 4. VS Code + Ubuntu/WSL에서 단계별 실행

ZIP 압축을 해제한 후 `Makefile`이 있는 디렉터리에서 아래 명령을 실행한다. 패키지에는 필요한 소스 코드와 QASM 파일이 포함되어 있다. 기존 실행 결과는 `results/`에 저장되어 있으며, 비교 재실행에는 새로운 `rerun/` 디렉터리를 사용한다.

### 단계 0 · 환경 설치 및 빌드

```bash
cd Capstone_Alpha_Research
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
export RAYON_NUM_THREADS=1 OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1
make -j2 all legacy
```

Python 3.10 이상과 C++17 컴파일러를 사용한다. Ubuntu의 `build-essential` 패키지는 `make`와 `g++`를 제공하고, `python3-venv` 패키지는 가상환경 기능을 제공한다. `make`를 실행하면 `build/research`와 `build/member`가 생성된다. 새로운 연구 코드는 `src/research_main.cpp`를 통해 실행되고, 팀원 버전은 `src/member_main.cpp`를 통해 실행된다.

### 단계 1 · 팀원 ZIP의 기준 결과 재현

```bash
python evaluate.py legacy --out rerun/legacy
```

`rerun/legacy/member_smoke.csv`를 연다. Tokyo20에서 ZIP에 포함된 `4gt13_92`는 5큐비트 회로이며 CX 30개를 포함한다. 팀원 버전은 라우팅 동작 0개를 반환한다. 각 행에서 `member_reported_actions = true_swaps + bridges` 관계를 확인한다.

**중간보고서의 기준값을 재현하려면 반드시 동일한 입력을 사용해야 한다.** 그룹의 중간보고 기준은 논리 큐비트 4개, 입력 CX 14개, Line5, 레이아웃 `[2, 1, 3, 0]`, Basic Routing, seed 42이며 결과는 CX 20개, depth 77이다. 이 기준을 재현하려면 중간보고에 사용한 정확한 회로와 스크립트를 동일한 설정으로 다시 실행해야 한다. 반면 E1은 팀원 C++ Mapper에서 Uniform W를 사용하고 SABRE 라우터를 고정하여 W의 독립적 효과를 측정한다. 두 실험은 서로 다른 질문에 답한다.

### 단계 2 · W, D, BRIDGE 및 출력 회로 검증

```bash
python evaluate.py smoke --out rerun/smoke --seeds 42 --repeats 1 --warmup 0
```

- `unit_checks.json`에는 의존성 레이어가 `[0,0,1,2]`인 CX 4개의 예제가 있다. W의 정수 값을 $10^{12}$로 나누면, 큐비트 쌍 $(0,1)$에 해당하는 CX 두 개는 Uniform에서 $W_{01}=2$, 레이어 기반 decay에서 $W_{01}=1+0.8^2=1.64$를 만든다. Line5의 거리는 $D_{ij}=|i-j|$이다. 이 검증에서 $\alpha_M=1$인 각 프로파일은 Uniform과 동일한 W 및 레이아웃을 반환한다.
- Line3의 양 끝에서 수행하는 CX 1개를 대상으로 하는 BRIDGE 시험에서는 BRIDGE 1개와 SWAP 0개가 생성된다. 출력 회로에는 물리 CX 4개가 있으며 연산자 동등성 검증을 통과한다.
- smoke의 `results.csv`에는 회로–토폴로지 조합 6개와 구성 9개에 대응하는 `PASS` 54행이 있어야 한다. `layouts.csv`에는 라우팅 전후 레이아웃이 기록된다. `FAIL`이 있는 행은 비교 수치를 사용하기 전에 먼저 `error` 열을 확인한다.

### 단계 3 · fit 데이터셋에서 계수 선택

```bash
python evaluate.py fit --out rerun/fit
```

`evaluate.py`는 `config/cases.json`에 정의된 회로–토폴로지 조합 12개를 사용한다. 네 가지 W 프로파일을 탐색하고, 같은 Greedy Uniform에 대해 $\alpha_R\in\{0.8,0.9,0.95,0.99\}$도 시험한다. 모든 사례를 유효하게 검증한 후보들은 기준선 대비 평균 $\Delta CX$를 우선으로, 다음으로 평균 $\Delta\text{depth}$를 기준으로 순위를 매긴다. decay 후보들 중 최적의 계수라도 Uniform보다 성능이 나쁠 수 있다. $\alpha=1$ 기준선은 차이를 평가하기 위해 유지한다.

- `ranking.csv`에는 순위, 유효한 사례 수, 두 평가지표의 차이가 기록된다.
- `selected.json`에는 $W^*$, $\alpha_R^*$, seed와 trial 설정, Qiskit 버전이 기록된다. 단계 4에서도 이 파일을 그대로 사용한다.
- 기본 설정은 seed 다섯 개 `42 0 1 2 3`, warm-up 1회, seed–구성 조합마다 측정 3회이다. SABRE 라우팅 한 번에는 trial 1개를 사용한다. SABRE 자체 레이아웃 선택은 해당 pass에 이미 포함된 시작 레이아웃 외에 무작위 레이아웃 trial 4개를 설정하여 사용한다.

### 단계 4 · 독립 평가 데이터셋(holdout)에서 검증

```bash
python evaluate.py holdout --selection rerun/fit/selected.json --out rerun/holdout
```

Line5, Grid6, Quito5, Tokyo20에 걸친 회로–토폴로지 조합 23개를 평가한다. 이 데이터셋은 회로 이름을 기준으로 fit 데이터셋과 분리되어 있다. 다만 일부 회로 계열의 변형이 두 데이터셋에 모두 존재하므로, 일반화에 관한 결론은 실제로 검사한 회로 범위로 한정해야 한다.

| `rerun/holdout/` 하위 경로 | 확인 방법 |
|---|---|
| `status.json`, `results.csv` | 유효 실행 횟수, 오류 유무, 회로 크기, seed, 레이아웃, SWAP, BRIDGE, CX, depth, 실행 시간을 확인한다. 결과를 종합할 때 각 회로–토폴로지 조합에 동일한 가중치를 준다. |
| `layouts.csv` | E1/E3/E5가 동일한 초기 레이아웃을 사용하고 E2/E4/E6도 동일한 초기 레이아웃을 사용하는지 확인한다. Router와 seed가 같을 때 레이아웃까지 같다면 결과 품질도 같아야 한다. |
| `paired.csv`, `summary.csv` | $\Delta=\text{후보 방식}-\text{기준 방식}$으로 정의하며 음수는 감소를 뜻한다. 토폴로지별 결과도 확인하여 평균값이 일부 사례의 비용 증가를 가리지 않는지 점검한다. |
| `run.json` | 실험 설정, QASM 해시, 실험군(arm), trial 수, seed를 확인하여 재현성을 확보한다. |

`pipeline.py`는 라우터의 실행 추적으로부터 1Q 및 CX 게이트를 모두 재구성하고, 유효한 연결 간선, 논리 큐비트 라인별 게이트 순서, 초기·최종 순열을 검증한다. 물리 큐비트가 최대 6개인 경우에는 전체 회로의 연산자 동등성도 비교한다. Tokyo20의 경우 각 큐비트 라인별 구조적 동등성을 확인하고, 최종 회로의 basis 및 연결 간선 적합성도 검사한다. 모든 품질지표는 동일한 basis 변환 수준에서 측정하며, 분해 이후 별도의 게이트 최적화는 수행하지 않는다.

SWAP과 BRIDGE의 개수는 분해 전에 계산한다. CX 수와 depth는 분해 후 회로에서 계산한다. SWAP 하나는 CX 3개로 구현한다. BRIDGE 하나는 멀리 떨어진 두 논리 큐비트 사이의 CX를 물리 CX 4개로 구현하므로 원래 CX 1개에 비해 CX 3개가 추가된다. readout이나 fidelity를 평가하려면 별도의 하드웨어 측정이 필요하다. Quito5는 스냅샷의 연결 그래프를 사용하고 Tokyo20은 ZIP에 포함된 그래프를 사용한다.

### 단계 5 · 시간 측정 결과 확정 및 종합

```bash
python summarize.py rerun/holdout
```

도구는 각 seed에 대해 반복 실행의 중앙값(median)을 먼저 구한 다음, 각 사례에서 seed 간 평균을 계산한다. 승/무/패는 평가지표별로 회로–토폴로지 조합 단위에서 계산한다. 반복 실행은 특히 결정론적인 C++ Router의 측정 안정성을 확인하는 데 사용한다. 다음 시간 관련 열을 확인한다.

- `weight_ms`, `greedy_ms`, `map_core_ms`: 내부 W 계산과 Greedy 실행 시간을 나타낸다. `map_core_ms`는 C++에서 D 생성, W 생성, 초기 레이아웃 선택에 걸린 시간을 합산한다.
- `map_wall_ms`: E1/E2에서는 전체 C++ wrapper를 포함하여 초기 레이아웃을 얻는 시간, E8에서는 SABRE의 레이아웃 선택 시간을 측정한다. E7에는 레이아웃 선택과 라우팅을 결합한 전체 시간이 기록된다.
- `total_ms`: 정규화된 회로에서 분해가 완료된 최종 회로까지 걸린 시간이다. 해당 구성에서는 C++ 프로세스 호출과 회로 재구성도 포함한다. `full_ms`에는 공통으로 측정한 입력 읽기 및 정규화 시간이 추가된다.
- C++ 내부의 `route_ms`와 레이아웃을 적용한 뒤 SABRE pass를 실행한 시간은 측정 범위가 서로 다르다. 파이프라인 간 속도 비교의 결론에는 `total_ms`를 사용한다. 계수 선택에 따른 비용을 논의할 때 fit 수행 시간은 별도로 보고한다.

Tokyo20 실험을 확장할 때는 동일한 `selected.json`과 seed를 사용한다.

```bash
python evaluate.py extended --selection rerun/fit/selected.json --out rerun/extended --repeats 1 --warmup 0
python evaluate.py stress --selection rerun/fit/selected.json --out rerun/stress --repeats 1 --warmup 0 --timeout 180
```

`extended`에는 나머지 중간 규모 QASM 11개가 포함된다. `stress`에는 ZIP 데이터셋 기준 입력 CX가 최대 224,028개인 매우 긴 회로 3개가 포함된다. 두 실험 분기를 따로 보고하고, 완료되지 않은 실행도 표에서 제외하지 않는다. SWAP만 사용하는 파이프라인도 추가로 평가하려면 fit 및 holdout 실행에 모두 `--no-bridge`를 추가하고 별도의 결과 폴더와 새로운 `selected.json`을 사용한다.

## 5. 기존 실행 결과

제공된 버전은 Linux 환경에서 g++ 14.2로 빌드하고 Python 3.12 및 Qiskit 2.5.2로 실행하였다. smoke 검증은 **54/54**, fit 데이터셋은 **3,600/3,600**, 주요 평가 데이터셋은 **4,485/4,485**를 통과하였다. 주요 평가의 4,485행 중 1,755행은 소형 회로에 대한 연산자 동등성 검증을, 2,730행은 Tokyo20에 대한 구조 검증을 수행하였다. 원본 소스에서 직접 빌드한 팀원 버전과 포팅한 버전은 §4의 단계 1에 있는 세 가지 재현 사례에서 초기 레이아웃, 최종 레이아웃, 동작 목록이 일치하였다. 상세 비교는 `results/legacy/source_parity.csv`를 참조한다.

fit 데이터셋에서 Mapper의 decay 방식 중 선택된 프로파일은 **CX 인덱스 기반 $w_M(t)=0.8^t$**이다. Router는 Greedy Uniform으로 생성한 초기 레이아웃에서 **$\alpha_R^*=0.8$**을 선택하였다. $W^*$는 평가 데이터셋의 **23개 사례 중 19개**에서 초기 레이아웃을 변경하였다.

### 5.1. 주요 평가 사례 23개의 회로 품질

각 숫자는 앞의 실험 방식에서 뒤의 실험 방식을 뺀 평균 차이를 나타낸다. 음수이면 앞의 실험 방식에 유리하다. 승/무/패는 각 사례에서 seed 간 평균을 취한 분해 후 CX 수를 기준으로 계산하였다.

| 비교 항목 | Δ SWAP | Δ CX | Δ depth | CX 승/무/패 |
|---|---:|---:|---:|---:|
| E2 − E1 · 동일한 SABRE에서 W만 변경 | −5.52 | **−16.57** | −23.63 | 9/8/6 |
| E5 − E3 · Greedy Uniform을 유지하고 Router alpha만 변경 | −42.83 | **−210.26** | −130.39 | 10/10/3 |
| E6 − E4 · decay W를 유지하고 Router alpha만 변경 | −62.52 | **−269.48** | −165.61 | 10/12/1 |
| E6 − E5 · alpha Router에 decay W 추가 | +2.30 | **+11.87** | −1.83 | 6/10/7 |
| E6 − E1 · 결합 방식과 Greedy Uniform + 고정 레이아웃 SABRE 비교 | −103.88 | **−148.07** | −204.93 | 19/2/2 |
| E6 − E7 · 결합 방식과 자체 레이아웃 선택 SABRE 비교 | −84.20 | **−89.03** | −148.85 | 15/2/6 |

- **W의 효과는 회로에 따라 다르다.** 동일한 SABRE에서 인덱스 기반 프로파일은 Tokyo20의 `z4_268`에서 CX를 212.4개 줄였으나 `wim_266`에서는 92.4개 늘렸다. 23개 사례에서 $\Delta CX$의 중앙값은 0이다. holdout의 평균 $\Delta CX$에서는 raw 레이어 기반 프로파일이 −28.88 CX로, raw 인덱스 기반 프로파일의 −16.57 CX보다 좋았다. 정규화된 두 프로파일은 이 데이터셋에서 모두 −0.76 CX를 기록하였다. fit에서 선정한 설정을 주요 결과로 유지하고, 레이어 기반 프로파일의 결과는 민감도 분석으로 제시한다.
- **구성 요소별 대조 실험에서 CX 감소에 가장 크게 기여한 것은 Router이다.** Tokyo20의 `sqn_258`과 `radd_250`은 E5 − E3의 평균 감소에 크게 기여하였으며 각각 −1,926 CX와 −1,149 CX를 기록하였다. E5 − E3의 중앙값은 0이다. E3는 E1보다 SWAP이 줄어드는 사례가 많았지만, BRIDGE 증가로 인해 평균 CX는 오히려 50.32개 증가하였다. 실제 구현 비용을 평가할 때는 분해 후 CX 수를 사용해야 한다.
- **결합 방식은 기준선보다 유리하지만 W 추가에는 상충 관계가 있다.** E5(Uniform + decay Router)는 전체 SABRE 대비 CX를 평균 100.90개 줄였으며 23개 중 13개 사례에서 승리하였다. E6은 23개 중 15개 사례에서 승리하였지만, E5 대비 평균 CX가 11.87개 증가하였다. 예를 들어 Tokyo20의 `z4_268`에서는 E6이 E5보다 CX를 222개 줄인 반면 `qft_20`에서는 156개 늘렸다.
- **효과는 토폴로지에 따라 달라진다.** E6 − E7의 차이는 Line5 **+2.00 CX**, Grid6 **−2.00 CX**, Quito5 **+1.60 CX**, Tokyo20 **−146.61 CX**이다. 전체 23개 사례의 중앙값은 **−3.00 CX**이다. Line5, Grid6, Quito5는 동일한 소형 QASM 세 개를 사용하므로 동일 입력에서 토폴로지의 영향을 비교할 수 있다.

### 5.2. Tokyo20 확장 검증

fit 데이터셋에서 선택한 계수를 변경하지 않고 Tokyo20 회로 11개를 추가로 실행하였다. **715/715**회 실행이 유효하였다. E6은 E7 대비 CX를 평균 **1,078.80개** 줄였고, 11개 중 10개 사례에서 CX 수가 더 적었다. 반면 E5와 비교하면 E6은 11개 중 6개 사례에서 승리했음에도 평균 CX가 **107.73개** 증가하였다. 이 게이트 수 결과는 주요 평가 결과를 보완한다. 이번 확장 실험은 seed당 1회만 실행했으므로 시간 결과는 주요 시간 비교 결론에 합치지 않는다. 매우 긴 회로 세 개에 대한 stress 실험은 아직 확장 단계로 남아 있다.

### 5.3. 실행 시간

| 평가 데이터셋에서의 측정 항목 | 평균 차이 | 사례별 차이의 중앙값 |
|---|---:|---:|
| E2 − E1 · C++ 내부의 D 생성, W 생성 및 Greedy | +0.016 ms | +0.011 ms |
| E2 − E1 · C++ wrapper를 포함한 레이아웃 획득 시간 | +0.086 ms | +0.042 ms |
| E8 − E2 · Greedy Decay 대비 SABRE 레이아웃 획득 시간 | +5.385 ms | −1.235 ms |
| E6 − E7 · 정규화 회로에서 최종 basis까지의 컴파일 시간 | +0.518 ms | +1.720 ms |
| E5 − E7 · 정규화 회로에서 최종 basis까지의 컴파일 시간 | +0.292 ms | +1.662 ms |

E8 − E2에서 평균과 중앙값의 부호가 다른 점은 레이아웃 선택 비용이 회로 크기에 따라 달라진다는 사실을 보여 준다. 전체 holdout 데이터셋에서 결합 방식의 총 실행 시간은 두 집계 방식 모두에서 전체 SABRE보다 길었다. 밀리초 단위의 작은 차이는 실행 장비, trial 수, 프로세스 호출 방식, 반복 측정의 안정성에 영향을 받는다. 재실행할 때는 `time python evaluate.py fit --out rerun/fit_timed`를 사용하여 파라미터 선택 과정 전체의 wall-clock 시간도 별도로 기록한다.

## 6. 연구 목표별 달성 정도

| 팀의 연구 목표 | 수집된 데이터에 따른 달성 상태 |
|---|---|
| 기존 Greedy 대비 $W(\alpha_M)$가 SWAP, CX, depth에 미치는 영향 측정 | **회로에 따라 효과가 달라짐을 검증함.** E2 − E1로 W의 효과를 분리하였다. Greedy Uniform은 동일한 C++ Mapper를 사용하는 알고리즘 기준선이다. 중간보고의 정확한 기준값은 §4에서 설명한 4큐비트 입력 및 중간보고 스크립트로 별도 재실행해야 한다. |
| Router의 거리 비용에 alpha를 적용했을 때의 영향 측정 | **검증 완료.** E5 − E3과 E6 − E4에서 look-ahead 가중 계수만 독립적으로 변경하였다. 이 데이터셋에서는 Router의 변경이 평균 CX 감소의 주된 원인이다. |
| $W(\alpha_M)$와 alpha Router를 결합한 효과 확인 | **품질 검증 완료, 결과는 혼합적임.** E6은 E1/E7보다 평균 CX와 depth를 줄였다. 그러나 E5에 비해서는 평균 CX가 증가하고 depth만 소폭 감소하였다. |
| 매핑 시간과 전체 컴파일 시간 비교 | **측정 완료.** 주요 측정에서는 전체 SABRE 대비 총 컴파일 시간을 단축한다는 목표를 달성하지 못했다. |
| 다양한 토폴로지 및 하드웨어에서 검증 | **4개 토폴로지의 연결 그래프 수준에서 완료.** Quito5와 Tokyo20 결과는 coupling graph 모델에 대한 결과이다. 실제 장비에서 측정했다는 내용을 보고서에 포함하려면 별도의 하드웨어 측정 데이터가 필요하다. |
| 팀원의 다중 패스 프로그램과 비교 | **3개 사례에서 재현성 검증 완료.** 주요 비교에는 단일 패스 C++ Router의 핵심 로직을 사용하였다. 새 결합 버전에서 $b$ 탐색과 순방향–역방향 레이아웃 개선을 활성화한 경우의 품질 측정은 추가 실험으로 남아 있다. |

기말보고서에서는 **SWAP 비용에 적용한 시간 가중치가 평균적인 라우팅 품질을 개선하였으나, Time-Decay Initial Mapping의 이점은 회로, 토폴로지, Router에 따라 달라진다**고 결론지을 수 있다. 평균 CX 수를 주요 평가지표로 사용할 경우, E5와 E6을 모두 결과 표에 제시하여 두 구성 사이의 성능 상충 관계를 명확히 보여 주는 것이 좋다.
