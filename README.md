# Mô phỏng Spanning Tree Protocol ET4230

K1–K5 và K7 dùng mạng 5 switch `b0`–`b4`. K6 thêm vòng `b4–b5–b6–b7–b4`, nâng mạng lên 8 switch. Các kịch bản dùng chung mô-đun `StpBridge`. Mỗi bridge lưu BPDU theo cổng, bầu root và tính vai trò cổng từ BPDU còn hiệu lực. Cấu hình `General` giữ mạng tam giác 10 s để kiểm thử nhanh. Xem [PLAN.md](PLAN.md) và [PARAMETERS.md](PARAMETERS.md).

## Chuẩn bị môi trường

Cài `opp_env` trên Linux hoặc Ubuntu WSL2. [Hướng dẫn cài opp_env](https://github.com/omnetpp/opp_env/blob/main/INSTALL.md)

Nếu cài `opp_env` trong venv Python tại `~/opp-venv`, chạy lệnh sau mỗi khi mở terminal Bash mới, trước khi dùng `opp_env`. Nếu đặt venv ở thư mục khác, thay đường dẫn cho phù hợp.

```bash
source ~/opp-venv/bin/activate
command -v opp_env
```

Nếu chưa có workspace, tạo một thư mục trong home của Linux hoặc Ubuntu WSL2. Ví dụ dưới đây dùng `~/omnet-ws`.

```bash
mkdir -p ~/omnet-ws
cd ~/omnet-ws
opp_env init
opp_env install omnetpp-6.5.0
```

### Cấu trúc thư mục

Clone hoặc giải nén toàn bộ dự án vào workspace. Đặt thư mục dự án cạnh thư mục cài OMNeT++, theo cấu trúc ví dụ sau.

```text
~/
├── opp-venv/                  # Ví dụ nơi cài opp_env nếu dùng venv Python
└── omnet-ws/                  # Workspace được khởi tạo bằng opp_env init
    ├── .opp_env_workspace/    # opp_env tự tạo khi khởi tạo workspace
    ├── omnetpp-6.5.0/         # Môi trường chạy do opp_env cài
    └── stp/                   # Thư mục dự án tải từ GitHub
        ├── README.md
        ├── Makefile           # Chạy make MODE=debug từ thư mục này
        ├── src/               # Mã C++, NED và định nghĩa thông điệp
        ├── simulations/
        │   ├── omnetpp.ini    # Cấu hình K1–K7
        │   └── check_stp.py   # Kiểm tra log mô phỏng
        └── out/               # Sinh khi build hoặc chạy kiểm thử hàng loạt
```

Từ terminal Linux hoặc Ubuntu WSL2, vào thư mục chứa cả `README.md` và `Makefile` của dự án. Nếu giải nén thành `stp-main`, dùng tên đó thay cho `stp` trong lệnh.

```bash
cd ~/omnet-ws/stp
```

## Build và chạy K1–K7

### 1. Kích hoạt môi trường và build

Chạy từ Bash trên Ubuntu WSL2. Ví dụ dưới đây dùng thư mục dự án `~/omnet-ws/stp` và venv `~/opp-venv`. Điều chỉnh tên thư mục theo nơi bạn đã đặt dự án và venv. Bỏ dòng `source` nếu `opp_env` đã có trên PATH.

```bash
source ~/opp-venv/bin/activate
cd ~/omnet-ws/stp
opp_env run omnetpp-6.5.0 -c 'make MODE=debug'
```

Build tạo `src/stp_dbg`. Các lệnh chạy bên dưới bắt đầu từ thư mục gốc của dự án. Cấu hình các kịch bản nằm trong [simulations/omnetpp.ini](simulations/omnetpp.ini).

### 2. Chạy từng kịch bản trên giao diện OMNeT++ (Qtenv)

Sao chép **một dòng** cho kịch bản muốn mở. `-r 0` chọn lượt chạy số 0. Trong cửa sổ Qtenv, bấm **Run** để bắt đầu mô phỏng hoặc **Step** để xem từng sự kiện. [Hướng dẫn Qtenv](https://docs.omnetpp.org/userguide/qtenv)

```bash
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K1 -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K2 -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K3 -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K4 -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K5 -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K6 -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K7_STP -r 0'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Qtenv -n .:../src -c K7_RSTP -r 0'
```

#### Khi chưa thấy cửa sổ Qtenv trên WSL

Nếu terminal dừng ở `Setting up Qtenv...` hoặc `Loading images...`, dùng **Alt+Tab** để tìm cửa sổ OMNeT++. Nếu vẫn chưa thấy, dừng lệnh hiện tại bằng **Ctrl+C** rồi thử mở K1 qua X11 của WSLg.

```bash
opp_env run omnetpp-6.5.0 -c 'cd simulations && QT_QPA_PLATFORM=xcb ../src/stp_dbg -u Qtenv -n .:../src -c K1 -r 0'
```

Nếu có lỗi kết nối display, kiểm tra cấu hình chạy ứng dụng Linux đồ họa trên WSL2. [Hướng dẫn WSLg của Microsoft](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)

### 3. Chạy bằng terminal (Cmdenv)

Chọn một dòng để xem trace của kịch bản trên terminal.

```bash
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K1 --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K2 --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K3 --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K4 --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K5 --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K6 --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K7_STP --cmdenv-express-mode=false'
opp_env run omnetpp-6.5.0 -c 'cd simulations && ../src/stp_dbg -u Cmdenv -n .:../src -c K7_RSTP --cmdenv-express-mode=false'
```

Để chạy đủ tám lượt và lưu log vào `out/check-stp/<cấu hình>/`, dùng lệnh sau. [Script kiểm tra kết quả](simulations/check_stp.py) đối chiếu cây chuyển tiếp và probe trong các cửa sổ ổn định. K1–K7 phát probe cho mọi cặp switch có hướng mỗi 0,1 s từ 8 s đến trước 23 s.

```bash
opp_env run omnetpp-6.5.0 -c '
set -eu
cd simulations
for config in K1 K2 K3 K4 K5 K6 K7_STP K7_RSTP; do
    log_dir=../out/check-stp/$config
    mkdir -p "$log_dir"
    ../src/stp_dbg -u Cmdenv -n .:../src -c "$config" \
        --cmdenv-express-mode=false --result-dir="$log_dir" \
        > "$log_dir/cmdenv.log" 2>&1
done
python3 check_stp.py ../out/check-stp > ../out/check-stp/summary.txt
cat ../out/check-stp/summary.txt
'
```

## Kết quả đã kiểm tra

Cả tám lượt Cmdenv đạt tiêu chí cây chuyển tiếp, hội tụ và kết nối trong các cửa sổ ổn định. Root port ghi theo switch ở đầu bên kia. Ba số đếm lần lượt là forwarding, blocking và down. Tỷ lệ probe được tính theo số gói đến đích trên số gói phát trong cửa sổ ổn định tương ứng. Lệnh chạy toàn bộ tạo `out/check-stp/summary.txt` và log của từng lượt.

| Kịch bản | Root | Root port theo thứ tự b0 trở đi | Số cổng | Hội tụ (s) | Ổn định đã quan sát (s) | Probe nhận/phát |
| --- | --- | --- | --- | ---: | ---: | ---: |
| K1 | b0 | —, b0, b1, b4, b0 | 10 / 2 / 0 | 4,01 | 20,99 | 3000/3000 |
| K2 | b3 | b4, b2, b3, —, b3 | 10 / 2 / 0 | 4,01 | 20,99 | 3000/3000 |
| K3 trước ngắt | b0 | —, b0, b1, b4, b0 | 10 / 2 / 0 | 4,01 | 6,49 | 500/500 |
| K3 sau tái hội tụ | b0 | —, b4, b1, b4, b0 | 9 / 1 / 2 | 3,51 | 10,99 | 1780/1780 |
| K4 | b0 | —, b4, b1, b4, b0 | 10 / 2 / 0 | 4,01 | 20,99 | 3000/3000 |
| K5 | b0 | —, b0, b1, b4, b0 | 9 / 1 / 0 | 4,01 | 20,99 | 3000/3000 |
| K6 | b0 | —, b0, b1, b4, b0, b4, b5, b4 | 17 / 3 / 0 | 4,01 | 20,99 | 8400/8400 |

K6 có 7 liên kết chuyển tiếp nối đủ 8 switch. Cổng `b6` hướng `b7` ở blocking, cùng với hai cổng blocking của K1.

Các lượt dùng hello 2 s, trễ 10 ms và thời lượng 25 s. Script đo hội tụ đến bản ghi `STP_STATE` thay đổi cuối của mỗi giai đoạn và kiểm tra còn ít nhất 6 s ổn định. Trong K3, `b1` chọn `b4` ngay tại 10,5 s. `b3` trở thành designated trên `b3–b2` tại 14,01 s khi nhận BPDU báo cost mới của `b2`, kết thúc tái hội tụ. K3 nhận 538/720 probe phát trong thời gian đổi cây và 2818/3000 probe trong toàn lượt.

### K7 — Khôi phục liên kết b0–b4

Cả hai lượt giữ hello 2 s, độ trễ 10 ms và thời lượng 25 s. Liên kết `b0–b4` down từ đầu và mở lại hai chiều tại 12 s. Cổng vừa mở chờ BPDU hợp lệ đầu tiên trước khi chuyển tiếp probe. STP gửi BPDU định kỳ. RSTP giản lược gửi thêm BPDU khi tuyến thay đổi hoặc liên kết đổi trạng thái. Mô hình này dùng chung quy tắc vai trò cổng và lược bỏ bắt tay proposal/agreement của RSTP chuẩn. [Tài liệu RSTP](https://www.cisco.com/c/en/us/support/docs/lan-switching/spanning-tree-protocol/24062-146.html)

| Kết quả | K7_STP | K7_RSTP |
| --- | ---: | ---: |
| Hội tụ ban đầu | 4,01 s | 0,03 s |
| Hai đầu `b0–b4` forwarding | 12,01 s | 12,01 s |
| Lần đổi trạng thái cuối | 14,01 s | 12,02 s |
| Tái hội tụ tính từ lúc khôi phục | 2,01 s | 0,02 s |
| Probe nhận được trước khôi phục | 800/800 | 800/800 |
| Probe nhận được trong lượt phát đúng 12 s | 19/20 | 17/20 |
| Probe nhận được ở các lượt phát tiếp theo trước khi tái hội tụ | 400/400 | Không có lượt phát |
| Probe nhận được sau tái hội tụ | 1780/1780 | 2180/2180 |
| Tổng probe nhận được | 2999/3000 | 2997/3000 |

Root vẫn là `b0`. Trước khi khôi phục, `b4` đi qua `b1` với cost 2 và mạng có 9 forwarding, 1 blocking, 2 down. Sau khi khôi phục, `b4` chuyển sang đường trực tiếp với cost 1. Hai lượt đạt cùng cây K1 với 4 liên kết chuyển tiếp, 10 forwarding và 2 blocking. RSTP giản lược cập nhật `b3` ở 12,02 s nhờ BPDU phát ngay từ `b4`. Ở lượt STP, `b3` nhận thông tin này ở 14,01 s.

Các probe rớt đều thuộc lượt phát ở 12 s. Một số bản sao đến cổng vừa chuyển sang blocking. Trace ghi 2 bản sao trùng ở STP và 3 ở RSTP giản lược, tất cả đều bị loại.

### Kiểm thử BPDU khi ngắt và khôi phục cùng liên kết

Sau khi build, chạy từ thư mục gốc để ngắt hai chiều `b0–b1` tại 10,002 s rồi mở lại tại 10,004 s. Probe dừng phát trước 10 s để đo riêng cây trước khi ngắt.

```bash
opp_env run omnetpp-6.5.0 -c '
set -eu
cd simulations
log_dir=../out/check-stp/restore-in-flight
mkdir -p "$log_dir"
../src/stp_dbg -u Cmdenv -n .:../src -c K1 \
    --cmdenv-express-mode=false --result-dir="$log_dir" \
    "--**.link01.failAt=10.002s" "--**.link01.restoreAt=10.004s" \
    "--**.probeEnd=10s" > "$log_dir/cmdenv.log" 2>&1
'
```

Trong `out/check-stp/restore-in-flight/cmdenv.log`, hai đầu nhận BPDU hợp lệ trước ngắt. Cây khớp K1 và nhận 400/400 probe, không có bản sao trùng. Hai BPDU phát ở 10 s đến tại 10,01 s và bị loại với `reason=before-restore`. Hai đầu chuyển tiếp lại ở 12,01 s, khi nhận BPDU mới.

Lượt thử K3 với `--**.link01.failAt=10.01s` xác nhận cả hai đầu loại BPDU đến đúng mốc ngắt. Lượt thử K1 với `--**.maxAge=0.5s` xác nhận bridge tính lại khi BPDU hết hạn rồi học lại khi BPDU mới đến.
