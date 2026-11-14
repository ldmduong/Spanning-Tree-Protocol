# Review và kiểm thử STP/RSTP K1–K7 · ET4230

## Kết luận

**K1–K6 và cả hai lượt K7 đều Đạt các tiêu chí về topology, root, vai trò cổng, cây chuyển tiếp và cửa sổ ổn định của kịch bản chính.** Build sạch và tám lượt Cmdenv đều hoàn tất. Cây sau hội tụ liên thông, không vòng và có đúng số cạnh yêu cầu ở cả 11 trạng thái đã kiểm tra. K1–K6 cần bật probe bằng tùy chọn dòng lệnh để đo kết nối theo [PLAN.md](PLAN.md#5-đo-lường-và-phân-tích). Sáu lượt bổ sung xác nhận các probe trong cửa sổ ổn định đều đến đích đúng một lần qua đường trên cây.

**Lỗi ảnh hưởng lớn nhất** nằm ở xử lý một liên kết có cả lịch ngắt lẫn lịch khôi phục. Bridge loại BPDU hợp lệ ngay từ đầu vì so thời điểm gửi với một mốc khôi phục còn ở tương lai. Lượt tái hiện cho thấy vòng chuyển tiếp tồn tại trước khi ngắt và bộ lọc probe loại 520 bản sao trùng. Cấu hình K3 chính chỉ đặt lịch ngắt, còn K7 đặt lịch mở một liên kết down từ đầu. Hai cấu hình chính vì vậy vẫn cho cây đúng.

K7 dùng chung một thuật toán vai trò cổng. Ở cấu hình chính, cả hai lượt có đường trực tiếp hai chiều từ **12,01 s**. BPDU phát theo sự kiện đưa lần đổi cổng cuối từ **14,01 s** xuống **12,02 s**, tương ứng tái hội tụ **2,01 s** và **0,02 s**. Tại lượt probe phát đúng 12 s, STP giản lược nhận 19/20 còn RSTP giản lược nhận 17/20. Kết quả đo thuộc về mô hình, lịch hello và lịch probe đã chạy.

## Phát hiện theo mức độ ảnh hưởng

### 1. Cao · Loại BPDU hợp lệ trước khi liên kết ngắt · Đã tái hiện

- **Vị trí và lệnh tái hiện.** [src/StpBridge.cc:135–154](src/StpBridge.cc#L135), [279–282](src/StpBridge.cc#L279). Chạy K1 với `'--**.link01.failAt=10.002s' '--**.link01.restoreAt=10.004s'`. Thêm `'--**.probeInterval=0.1s' '--**.probeEnd=10s' --record-eventlog=true` để đo dữ liệu trước khi ngắt.
- **Thực tế.** Hai đầu loại BPDU tại 0,01 s và lặp lại đến 8,01 s dù link vẫn up. Tại 2,01 s, b1 chọn b4 cost 2 và để cổng về b0 ở designated/forwarding. Cả hai đầu của ba cạnh b0–b1, b0–b4 và b1–b4 đều forwarding trước sự cố. Probe phát trong [8, 10) đến đích 400/400, nhưng tạo 520 lượt loại vì trùng. Ví dụ bản sao của probe b0 → b1 `seq=0` đến b1 lần hai qua b4 tại 8,02 s. [Log BPDU](out/review-k1-k7/restore-in-flight/cmdenv.log#L45) · [Log probe](out/review-k1-k7/restore-in-flight-probe/cmdenv.log#L278) · [Đường đi từ eventlog](out/review-k1-k7/inspect.txt)

~~~text
STP_DROP t=0.01 bridge=b1 port=0 reason=before-restore
STP_DROP t=0.01 bridge=b0 port=0 reason=before-restore
PROBE_RECV t=8.01 src=0 dst=1 seq=0
PROBE_DROP t=8.02 bridge=b1 src=0 dst=1 seq=0 reason=duplicate
~~~

- **Kết quả đúng và đề xuất.** Trước 10,002 s, BPDU hợp lệ qua link đang up phải được học, b1 phải dùng đường trực tiếp cost 1 và cây phải khớp K1. BPDU phát lúc 10 s đến lúc 10,01 s, sau khi mở lại, vẫn phải bị loại. Chỉ áp dụng so sánh thời điểm phát với mốc khôi phục sau khi đã tới mốc đó hoặc dùng thời điểm mở gần nhất thực sự xảy ra. Giữ hai phép thử đối chứng cho BPDU trước ngắt và BPDU đang truyền qua lần khôi phục.

### 2. Trung bình · K7 có probe quay về nguồn và mất gói đúng lúc đổi cây · Đã tái hiện

- **Vị trí và tái hiện.** [src/StpBridge.cc:35–37](src/StpBridge.cc#L35), [182–206](src/StpBridge.cc#L182), [320–343](src/StpBridge.cc#L320). Chạy nguyên cấu hình `K7_STP` và `K7_RSTP` với eventlog rồi theo dõi lượt phát ở 12 s.
- **Thực tế.** Probe b4 → b2 `seq=162` có một bản sao quay về nguồn theo b4@12 → b1@12,01 → b0@12,02 → b4@12,03 trong cả hai lượt. Bản sao bị loại, không có lần nhận đích lặp. RSTP giản lược đổi root port b3 khi các probe đã rời nguồn, làm mất thêm hai ID so với STP giản lược. Phần K7 bên dưới ghi từng nhánh bị chặn. [STP](out/review-k1-k7/K7_STP/cmdenv.log#L5694) · [RSTP](out/review-k1-k7/K7_RSTP/cmdenv.log#L5752) · [Đường đi](out/review-k1-k7/inspect.txt)
- **Kết quả đúng và đề xuất.** Cây ổn định phải chuyển probe theo một đường duy nhất. Điều kiện này đạt ở hai phía của sự kiện. Trong chuyển tiếp, cần báo riêng ID mất và bản sao lặp như kế hoạch. Nếu mục tiêu là ngăn đường quay lại khi đổi cây, mô hình cần phối hợp trạng thái cổng giữa hai đầu và kiểm thử probe đang truyền trước khi đổi sang forwarding. Một BPDU đầu tiên hiện chỉ mở cổng cục bộ, chưa đồng bộ các nhánh còn lại.

### 3. Trung bình · Root đã bị cô lập vẫn được quảng bá đến khi BPDU cũ hết hạn · Đã tái hiện ngoài cấu hình chính

- **Vị trí và lệnh tái hiện.** [src/StpBridge.cc:155–171](src/StpBridge.cc#L155), [232–246](src/StpBridge.cc#L232), [295–302](src/StpBridge.cc#L295). Chạy K3 thêm `'--**.link04.failAt=10.5s' --sim-time-limit=45s` để cô lập b0.
- **Thực tế.** Tại 10,5 s, b1 và b4 chọn nhau làm đường tới b0 qua các BPDU còn hạn. Hai cổng cùng là root/forwarding và cùng quảng bá tuyến phụ thuộc vào phía kia. Cost ở b1 tăng từ 2 lên 10 trong khi root vẫn là b0. Các mục BPDU hết hạn ở 28 s, sau đó thành phần b1–b4 ổn định với root b1 ở 30,01 s. Thời hạn tuyệt đối chặn việc gia hạn thông tin cũ vô hạn, nhưng còn một giai đoạn dùng tuyến không tới root. [Cổng b4 và b1 lúc ngắt](out/review-k1-k7/isolate-root/cmdenv.log#L223) · [Cost tăng và hết hạn](out/review-k1-k7/isolate-root/cmdenv.log#L459)
- **Kết quả đúng và đề xuất.** Sau khi b0 bị cô lập, thành phần b1–b4 phải loại tuyến phụ thuộc vòng và bầu b1, còn b0 là root của thành phần riêng. Nếu cần phục hồi sớm cho tình huống này, bổ sung cơ chế rút tuyến hoặc vô hiệu hóa quảng bá phụ thuộc vào nhánh vừa mất, rồi kiểm tra lại bằng ca cô lập b0. Hạ `maxAge` chỉ rút ngắn thời gian tồn tại của thông tin cũ và có thể gây hết hạn giữa hai hello.

### 4. Trung bình · Cấu hình mặc định K1–K6 thiếu phép đo dữ liệu theo kế hoạch · Đã tái hiện

- **Vị trí và tái hiện.** [PLAN.md:69–74](PLAN.md#L69), [src/StpNet.ned:26–28](src/StpNet.ned#L26), [simulations/omnetpp.ini:5–36](simulations/omnetpp.ini#L5). Chạy sáu cấu hình nguyên bản. Các tệp `.sca` ghi `probeInterval 0s` và log có 0 sự kiện `PROBE_SEND`. **Không đạt** mục đo kết nối theo kế hoạch khi chỉ chạy cấu hình mặc định K1–K6. [Ví dụ tham số thực tế K3](out/review-k1-k7/K3/K3-%230.sca) · [Log K3](out/review-k1-k7/K3/cmdenv.log)
- **Kết quả đúng và đề xuất.** Lệnh thu kết quả cần bật probe cho 20 hoặc 56 cặp có hướng và đo K3 cả hai phía sự kiện. Sáu lượt `probe-K1` đến `probe-K6` với override đã xác minh được chỉ tiêu này. Có thể đưa đúng tùy chọn bật probe vào lệnh thu log của README và giữ các cấu hình chỉ đo trạng thái cổng cho kiểm thử nhanh. [Kết quả bổ sung](out/review-k1-k7/inspect.txt)

### 5. Thấp · Script hiện có bỏ qua mất probe trong một phần cửa sổ tái hội tụ · Đã tái hiện trên bản sao log

- **Vị trí và tái hiện.** [simulations/check_stp.py:82–93](simulations/check_stp.py#L82). Bỏ đúng dòng `PROBE_RECV t=12.11 src=0 dst=1 seq=164` trong bản sao log K7-STP, rồi chạy `python3 simulations/check_stp.py <thư-mục-log-bản-sao>`.
- **Thực tế.** Script trả mã 0, tổng đổi từ 2999/3000 thành 2998/3000. Gói phát lúc 12,1 s nằm giữa lượt phát đúng mốc khôi phục và lần đổi cuối. Script chỉ xác nhận trước sự kiện, lượt phát tại sự kiện và sau hội tụ. [Diff một dòng và kết quả chạy](out/review-k1-k7/check-tools.log)
- **Kết quả đúng và đề xuất.** Thống kê phải bao phủ toàn bộ ID theo thời điểm phát. Thêm cửa sổ còn lại trong lúc đổi cây và liệt kê các ID thiếu kèm lúc phát. Khi dự án chốt kỳ vọng cho trace tất định, thêm kiểm tra các ID được phép mất trong ca K7.

### 6. Thấp · Điểm chạy phụ trỏ tới sai binary sau build debug · Đã tái hiện

- **Vị trí và lệnh tái hiện.** [simulations/run:2–3](simulations/run#L2), [README.md:5–23](README.md#L5). Sau `make MODE=debug`, chạy `sh simulations/run -u Cmdenv -c K1` trong bản sao build.
- **Thực tế.** `simulations/run: 3: ../src/stp: not found`, mã thoát 127. `src/stp_dbg` đã được build và lệnh trực tiếp trong README chạy được. [Log kiểm tra](out/review-k1-k7/check-tools.log)
- **Kết quả đúng và đề xuất.** Đồng bộ script với binary debug được README hướng dẫn, hoặc bỏ điểm chạy phụ nếu chỉ duy trì lệnh trong README.

## Kỳ vọng tính từ topology và kết quả từng kịch bản

Lấy root nhỏ nhất theo `(priority, bridgeId)`. Từ các cạnh đang hoạt động, tính cost ngắn nhất về root theo hướng từ bridge đang xét về láng giềng. Chọn root port bằng cost cộng dồn rồi đến định danh bridge và cổng phát khi hòa. Trên mỗi cạnh, đầu có vector quảng bá tốt hơn là designated. Nếu đầu kia dùng cạnh đó về root thì là root/forwarding, nếu không thì là alternate/blocking. Cạnh bị ngắt có cả hai đầu disabled/down. Tính từ [topology NED](src/StpNet.ned#L63), [cấu hình](simulations/omnetpp.ini) và đối chiếu từng cổng bằng [script kiểm tra độc lập](out/review-k1-k7/inspect.py).

| Kịch bản | Kỳ vọng tính từ các cạnh và cost |
| --- | --- |
| K1 | Tất cả cùng priority, b0 thắng bằng ID. b2 và b3 đều cost 2, nên b2 thắng designated trên cạnh b2–b3. b1 và b4 cùng cost 1, b1 thắng designated trên dây chéo. |
| K2 | Priority 4096 của b3 thắng 32768 của b0. b1 có hai đường cost 2 qua b2 và b4. Hai bridge gửi cùng priority, ID 2 thắng ID 4. Qua b0 có cost 3. Trên b0–b1 hai đầu cùng cost 2, b0 thắng designated. b4 thắng designated trên b1–b4 vì cost 1. |
| K3 | Trước ngắt khớp K1. Bỏ b0–b1, b1 có cost 2 qua b4, b2 cost 3 qua b1 hoặc b3, chọn b1 theo ID. b3 cost 2 thắng designated trên b2–b3. |
| K4 | Đường trực tiếp b1 → b0 cost 3, qua b4 cost 2. b0 thắng designated trên cạnh b0–b1. Các cạnh còn lại có vai trò như K3 sau ngắt. |
| K5 | Giữ năm switch và bỏ b1–b4. Các đường ngắn nhất của K1 vẫn còn. Chỉ cổng b3 về b2 bị chặn. |
| K6 | Có tám switch, mười liên kết. b5 và b7 cost 2 qua b4. b6 có hai đường cost 3 qua b5 và b7, chọn b5 theo ID. b7 thắng designated trên b6–b7 do cost thấp hơn. |
| K7-STP và K7-RSTP | Trước khôi phục, b4 cost 2 qua b1, b3 cost 3 qua b2 hoặc b4 và chọn b2 theo ID. b4 thắng designated trên b3–b4 vì cost 2. Sau khôi phục, b4 có đường trực tiếp cost 1, b3 có đường cost 2 qua b4 và toàn mạng trở về cây K1. |

Trong bảng sau, dãy **láng giềng/cost** theo thứ tự b0, b1, …, b7. Dấu `–/0` là root không có root port. Ký hiệu `bX→bY` chỉ một đầu cổng cục bộ của bX. Các đầu cổng trong dãy root port là root/forwarding. Các đầu ghi chặn là alternate/blocking, hai đầu cạnh ghi down là disabled/down. Mọi đầu còn lại là designated/forwarding. Như vậy bảng xác định vai trò và trạng thái của từng đầu cổng, kể cả đầu designated trên một cạnh không dùng để chuyển tiếp hai chiều.

| Kịch bản và giai đoạn | Trạng thái | Root | Láng giềng root port/cost theo thứ tự bridge | Cổng chặn và cạnh down | Forwarding / blocking / down | Cạnh cả hai đầu forwarding | Đổi cuối (s) | Hội tụ (s) | Cửa sổ ổn định (s) |
| --- | --- | --- | --- | --- | --- | --- | ---: | ---: | --- |
| K1 | Đạt | b0 | –/0, b0/1, b1/2, b4/2, b0/1 | b3→b2, b4→b1 | 10 / 2 / 0 | b0–b1, b0–b4, b1–b2, b3–b4 | 4,01 | 4,01 | [4,01; 25), 20,99 |
| K2 | Đạt | b3 | b4/2, b2/2, b3/1, –/0, b3/1 | b1→b0, b1→b4 | 10 / 2 / 0 | b0–b4, b1–b2, b2–b3, b3–b4 | 4,01 | 4,01 | [4,01; 25), 20,99 |
| K3 trước ngắt | Đạt | b0 | –/0, b0/1, b1/2, b4/2, b0/1 | b3→b2, b4→b1 | 10 / 2 / 0 | b0–b1, b0–b4, b1–b2, b3–b4 | 4,01 | 4,01 | [4,01; 10,5), 6,49 |
| K3 sau ngắt | Đạt | b0 | –/0, b4/2, b1/3, b4/2, b0/1 | b2→b3. Down b0–b1 | 9 / 1 / 2 | b0–b4, b1–b2, b1–b4, b3–b4 | 14,01 | 3,51 | [14,01; 25), 10,99 |
| K4 | Đạt | b0 | –/0, b4/2, b1/3, b4/2, b0/1 | b1→b0, b2→b3 | 10 / 2 / 0 | b0–b4, b1–b2, b1–b4, b3–b4 | 4,01 | 4,01 | [4,01; 25), 20,99 |
| K5 | Đạt | b0 | –/0, b0/1, b1/2, b4/2, b0/1 | b3→b2 | 9 / 1 / 0 | b0–b1, b0–b4, b1–b2, b3–b4 | 4,01 | 4,01 | [4,01; 25), 20,99 |
| K6 | Đạt | b0 | –/0, b0/1, b1/2, b4/2, b0/1, b4/2, b5/3, b4/2 | b3→b2, b4→b1, b6→b7 | 17 / 3 / 0 | b0–b1, b0–b4, b1–b2, b3–b4, b4–b5, b4–b7, b5–b6 | 4,01 | 4,01 | [4,01; 25), 20,99 |
| K7-STP trước mở | Đạt | b0 | –/0, b0/1, b1/2, b2/3, b1/2 | b3→b4. Down b0–b4 | 9 / 1 / 2 | b0–b1, b1–b2, b1–b4, b2–b3 | 4,01 | 4,01 | [4,01; 12), 7,99 |
| K7-STP sau mở | Đạt | b0 | –/0, b0/1, b1/2, b4/2, b0/1 | b3→b2, b4→b1 | 10 / 2 / 0 | b0–b1, b0–b4, b1–b2, b3–b4 | 14,01 | 2,01 | [14,01; 25), 10,99 |
| K7-RSTP trước mở | Đạt | b0 | –/0, b0/1, b1/2, b2/3, b1/2 | b3→b4. Down b0–b4 | 9 / 1 / 2 | b0–b1, b1–b2, b1–b4, b2–b3 | 0,03 | 0,03 | [0,03; 12), 11,97 |
| K7-RSTP sau mở | Đạt | b0 | –/0, b0/1, b1/2, b4/2, b0/1 | b3→b2, b4→b1 | 10 / 2 / 0 | b0–b1, b0–b4, b1–b2, b3–b4 | 12,02 | 0,02 | [12,02; 25), 12,98 |

Duyệt các cạnh hai đầu forwarding từ root tới đủ 5 hoặc 8 bridge. Các đồ thị tương ứng có 4 hoặc 7 cạnh nên liên thông và không vòng. Mốc đổi cuối lấy theo thứ tự sự kiện của toàn bộ `STP_STATE` khi root, priority, cost, root port, vai trò hoặc trạng thái thay đổi. Thời gian hội tụ là mốc đó trừ thời điểm bắt đầu giai đoạn. Tất cả các cửa sổ trên dài ít nhất 6 s, tương đương ba chu kỳ hello 2 s. K3 trước ngắt và hai lượt K7 trước mở đều hoàn thành đủ 6 s trước mốc sự kiện. Ở K3 và K7-STP, trace có đủ năm sự kiện hello mỗi mốc 6, 8, 10 s trước sự kiện và 16, 18, 20 s sau sự kiện. Các mốc tương ứng của K7-RSTP là 2, 4, 6 s và 14, 16, 18 s. [Kết quả script gốc](out/review-k1-k7/summary.txt) · [Đối chiếu độc lập](out/review-k1-k7/inspect.txt)

### Dấu vết K1–K6

- **K1 và K5.** Ở 4,01 s, b3 ghi `root=0 cost=2 rootPort=1 port=0:b2:alternate:blocking`. K1 có thêm `port=2:b1:alternate:blocking` tại b4 lúc 2,01 s. K5 có 10 đầu cổng, không có hai đầu dây chéo. [K1](out/review-k1-k7/K1/cmdenv.log#L88) · [K5](out/review-k1-k7/K5/cmdenv.log#L111)
- **K2.** Tại b1, hai BPDU ở 4,01 s lần lượt có `root=3 priority=4096 cost=1 sender=2` và `root=3 priority=4096 cost=1 sender=4`. Kết quả `cost=2 rootPort=1` chọn b2. BPDU từ b0 báo cost 2 nên đường qua đó có tổng 3. [K2](out/review-k1-k7/K2/cmdenv.log#L111)
- **K3 chính.** BPDU trực tiếp vào b1 lúc 10,01 s có `sender=0 cost=0 expires=30`. Cùng lúc, b1 còn lưu BPDU qua cổng 2 với `sender=4 cost=1 expires=28`. Ở 10,5 s, hai đầu cổng 0 báo down. b1 đổi sang cổng 2 cost 2 ngay lúc đó, dù BPDU qua cổng cũ còn thời hạn trên bản tin. Đến 12,01 s, b4 học cost 2 từ b1 và mở cổng hướng b1, còn b2 học cost 3 và chặn hướng b3. Đến 14,01 s, b3 nhận `sender=2 cost=3` và đổi cổng hướng b2 sang designated/forwarding. [K3](out/review-k1-k7/K3/cmdenv.log#L199)

~~~text
STP_LINK_DOWN t=10.5 bridge=b0 peer=b1 port=0
STP_LINK_DOWN t=10.5 bridge=b1 peer=b0 port=0
STP_STATE t=10.5 bridge=b1 root=0 priority=32768 cost=2 rootPort=2 port=0:b0:disabled:down port=1:b2:designated:forwarding port=2:b4:root:forwarding
STP_BPDU t=14.01 bridge=b3 port=0 root=0 priority=32768 cost=3 sender=2 senderPriority=32768 senderPort=1 expires=28
~~~

- **BPDU đang truyền ở K3.** Lượt chính gửi ở 10 s, tới 10,01 s, nên không có BPDU trên b0–b1 đúng lúc 10,5 s. Lượt chẩn đoán đổi `failAt=10.01s` giữ các tham số còn lại. Hai sự kiện down xử lý trước hai BPDU đến cùng thời điểm. Lượt này dùng để thử thứ tự sự kiện, không dùng thay mốc K3 chính. [Trace tại mốc ngắt](out/review-k1-k7/in-flight/cmdenv.log#L199)

~~~text
STP_DROP t=10.01 bridge=b1 port=0 reason=down
STP_DROP t=10.01 bridge=b0 port=0 reason=down
~~~

- **K4 và chiều áp dụng cost.** Tệp kết quả ghi `pathCost 3` cho cả hai kênh `link01`. b1 nhận BPDU b0 có cost quảng bá 0 lúc 0,01 s và tính cost 3 trên cổng 0. Lúc 2,01 s, b1 chọn b4 cost 2 và cổng 0 vẫn nhận BPDU dù ở alternate/blocking. Trong lượt K1 với chỉ kênh phát của b1 tăng lên 3, b1 cũng đổi sang b4. Khi chỉ kênh phát của b0 tăng lên 3, b1 giữ đường trực tiếp cost 1. Mã đọc cost ở `ethg$o[i]` của bridge đang chọn đường và cộng vào vector nhận trên cổng cùng chỉ số. [K4](out/review-k1-k7/K4/cmdenv.log#L45) · [Cost hai chiều](out/review-k1-k7/K4/K4-%230.sca) · [Chỉ tăng b1 → b0](out/review-k1-k7/cost-b1-out/cmdenv.log#L104) · [Chỉ tăng b0 → b1](out/review-k1-k7/cost-b0-out/cmdenv.log#L46) · [Mã](src/StpBridge.cc#L87)
- **K6.** Tại 4,01 s, b6 ghi `root=0 cost=3 rootPort=0 port=0:b5:root:forwarding port=1:b7:alternate:blocking`. Có 8 bridge, 20 đầu cổng, 10 liên kết vật lý và 7 liên kết hai đầu forwarding. [K6](out/review-k1-k7/K6/cmdenv.log#L212)

### Chuyển tiếp dữ liệu K1–K6

Sáu lượt bổ sung giữ nguyên toàn bộ chuỗi `STP_STATE` của lượt chính. Chỉ bật phát probe mỗi 0,1 s từ 8 s đến trước 23 s và ghi eventlog. Theo dõi ID `(src, seq)` qua `CM`, `CL`, `ES` và sự kiện nhận `E` trong `.elog`. Kiểm tra từng lần nhận đúng đích, đúng một lần và trong giai đoạn ổn định đi đúng đường duy nhất trên cây. [Định dạng eventlog OMNeT++](https://doc.omnetpp.org/omnetpp/manual/) · [Kết quả theo từng lượt](out/review-k1-k7/inspect.txt)

| Lượt | Nhận / phát | Kết nối trong các cửa sổ ổn định |
| --- | ---: | --- |
| probe-K1 | 3000/3000 | 20/20 cặp mỗi lượt phát |
| probe-K2 | 3000/3000 | 20/20 cặp mỗi lượt phát |
| probe-K3 | 2818/3000 | 500/500 trước ngắt và 1780/1780 từ sau hội tụ, đủ 20/20 cặp mỗi lượt |
| probe-K4 | 3000/3000 | 20/20 cặp mỗi lượt phát |
| probe-K5 | 3000/3000 | 20/20 cặp mỗi lượt phát |
| probe-K6 | 8400/8400 | 56/56 cặp mỗi lượt phát |

K3 mất 12 probe mỗi lượt phát từ 10,5 đến 11,9 s và thêm hai probe b4 → b1, b4 → b2 ở lượt 12 s. Từ lúc ngắt đến khi b4 mở cổng hướng b1 ở 12,01 s, b1 và b2 bị tách khỏi nhóm b0, b3, b4 trên đồ thị forwarding. Ngay lúc phát ở 12 s, b4 vẫn chưa dùng được dây chéo. Tổng trong `[10,5; 14,01)` là 538/720. Ví dụ b1 → b0 `seq=100` phát ở 10,5 s bị loại tại b4 ở 10,51 s. Cặp b0 → b1 đi trực tiếp trước ngắt, còn ở 18 s đi b0 → b4 → b1 và nhận lúc 18,02 s. Sáu lượt không có bản sao trùng hay chạm giới hạn hop. Các bản sao bị loại ở cổng blocking trong lúc flood được đếm riêng với số ID không đến đích. [Probe K3 quanh sự cố](out/review-k1-k7/probe-K3/cmdenv.log#L4036) · [Probe K3 sau sự cố](out/review-k1-k7/probe-K3/cmdenv.log#L13038) · [Eventlog K3](out/review-k1-k7/probe-K3/K3-%230.elog)

## K7 · So sánh trực tiếp hai lượt

Đối chiếu các tham số thực tế ghi trong `.sca` và lịch phát trong eventlog. Cả hai dùng `stp.StpFive`, b0–b4 có ID 0–4 và priority 32768, đủ sáu liên kết vật lý, cost 1 và trễ 10 ms ở cả hai chiều mỗi liên kết. Hello 2 s, max age 20 s, thời lượng 25 s. Hai kênh của `link04` có `failAt=-1s` và `restoreAt=12s`, tương ứng down từ đầu rồi mở ở 12 s. Lịch probe có 150 lượt từ 8 đến 22,9 s, mỗi lượt đủ 20 cặp có hướng, tổng 3000. Các tham số mô hình chỉ khác `rapid=false` và `rapid=true`. Các giá trị đo trùng kế hoạch. [K7-STP .sca](out/review-k1-k7/K7_STP/K7_STP-%230.sca) · [K7-RSTP .sca](out/review-k1-k7/K7_RSTP/K7_RSTP-%230.sca) · [Cấu hình kế thừa](simulations/omnetpp.ini#L29) · [Kế hoạch tham số](PARAMETERS.md#L32)

| Mốc | K7-STP | K7-RSTP |
| --- | --- | --- |
| Khởi tạo | Hai đầu b0–b4 down. b0 thắng bầu root. b4 chọn b1 cost 2. Cây ổn định từ 4,01 s. | Cùng cây, BPDU theo sự kiện lan qua các hop. b3 chặn hướng b4 ở 0,03 s, hoàn tất cây. |
| 12,00 s | Hai đầu up ở cùng thời điểm mô phỏng, báo designated/blocking do đang chờ BPDU hợp lệ. Timer cổng chạy trước hello. Hello ở 12 s gửi BPDU qua b0–b4. | Cùng trình tự. Do hello đã được lập lịch đúng 12 s, `rapid` không gửi thêm tại mốc này. |
| 12,01 s | b4 nhận BPDU b0 báo cost 0, đổi từ cổng 2 hướng b1 sang cổng 1 hướng b0, cost 2 → 1, đồng thời chặn hướng b1. b0 nhận BPDU từ b4 còn báo cost 2, giữ root và mở cổng designated. Hai đầu đường trực tiếp cùng forwarding. | Cùng hai thay đổi. b4 phát ngay BPDU mới với cost 1 do tuyến vừa đổi. |
| 12,02 s | b3 vẫn chọn b2. b3 đã nhận thông tin cost 2 cũ của b4 tại 12,01 s. Hai đường qua b2 và b4 vẫn cùng cost 3 theo thông tin cục bộ. | b3 nhận BPDU mới từ b4 và đổi sang cổng 1, cost 3 → 2. Cổng 0 hướng b2 thành alternate/blocking. Đây là thay đổi cuối. |
| 14,01 s | b3 nhận BPDU cost 1 do b4 gửi ở hello 14 s và thực hiện cùng thay đổi. Đây là thay đổi cuối. | BPDU định kỳ xác nhận cây hiện tại. |

Trích các bản ghi liền với sự kiện đổi cổng. [K7-STP mở link](out/review-k1-k7/K7_STP/cmdenv.log#L5523) · [K7-STP đổi cổng cuối](out/review-k1-k7/K7_STP/cmdenv.log#L8659) · [K7-RSTP mở link và phát thêm](out/review-k1-k7/K7_RSTP/cmdenv.log#L5565) · [K7-RSTP đổi cổng cuối](out/review-k1-k7/K7_RSTP/cmdenv.log#L5679)

~~~text
STP_LINK_UP t=12 bridge=b0 peer=b4 port=1
STP_LINK_UP t=12 bridge=b4 peer=b0 port=1
STP_STATE t=12.01 bridge=b4 root=0 priority=32768 cost=1 rootPort=1 port=0:b3:designated:forwarding port=1:b0:root:forwarding port=2:b1:alternate:blocking
STP_STATE t=12.01 bridge=b0 root=0 priority=32768 cost=0 rootPort=-1 port=0:b1:designated:forwarding port=1:b4:designated:forwarding
~~~

~~~text
K7-STP
STP_BPDU t=14.01 bridge=b3 port=1 root=0 priority=32768 cost=1 sender=4 senderPriority=32768 senderPort=0 expires=32
STP_STATE t=14.01 bridge=b3 root=0 priority=32768 cost=2 rootPort=1 port=0:b2:alternate:blocking port=1:b4:root:forwarding

K7-RSTP
STP_TRIGGER t=12.01 bridge=b4
STP_BPDU t=12.02 bridge=b3 port=1 root=0 priority=32768 cost=1 sender=4 senderPriority=32768 senderPort=0 expires=32
STP_STATE t=12.02 bridge=b3 root=0 priority=32768 cost=2 rootPort=1 port=0:b2:alternate:blocking port=1:b4:root:forwarding
~~~

**Cơ chế đo được.** Cả hai chế độ phát BPDU định kỳ ở tất cả cổng đang up và dùng cùng vector, bộ định thời hết hạn, điều kiện forwarding và hàng đợi sự kiện. `rapid` phát thêm khi root/cost/root port hoặc trạng thái link đổi, miễn hello kế tiếp không ở cùng thời điểm. Một đổi vai trò đơn lẻ không kích hoạt phát thêm. Cổng vừa mở chờ BPDU hợp lệ đầu tiên rồi xét lại vai trò. Phiên bản STP lược bỏ listening, learning, forward delay và chuyển tiếp BPDU theo nhịp do root dẫn. Phiên bản rapid dùng cùng định dạng BPDU, lược bỏ proposal/agreement, đồng bộ cổng, trạng thái chuẩn và xử lý thay đổi bảng MAC. Probe là flood có bộ lọc trùng và giới hạn hop. `ned.DelayChannel` chỉ mô hình hóa trễ lan truyền. Do đó phép so sánh đo ảnh hưởng của phát BPDU theo sự kiện trong mô hình này. [Mã phát BPDU](src/StpBridge.cc#L202) · [Timer hello](src/StpBridge.cc#L250) · [Định nghĩa BPDU](src/Bpdu.msg#L15) · [Kênh truyền](src/StpNet.ned#L55) · [Cơ chế RSTP và forward delay trong tài liệu Cisco](https://www.cisco.com/c/en/us/support/docs/lan-switching/spanning-tree-protocol/24062-146.html)

**Đối chứng pha hello.** Chạy bổ sung cả hai chế độ với `restoreAt=12.5s`. STP mở cả hai đầu lúc 14,01 s và đổi lần cuối lúc 16,01 s, tức 3,51 s sau khôi phục. Rapid kích hoạt BPDU tại 12,5 s, mở cả hai đầu lúc 12,51 s và đổi lần cuối lúc 12,52 s, tức 0,02 s. Việc hai đường trực tiếp cùng bắt đầu forwarding ở 12,01 s trong cấu hình chính đến từ mốc mở trùng hello. [Đối chứng STP](out/review-k1-k7/phase-STP/cmdenv.log#L8246) · [Đối chứng RSTP](out/review-k1-k7/phase-RSTP/cmdenv.log#L6256)

### Kiểm tra nhãn cổng trên Qtenv khi khôi phục K7

Trước khi sửa, Qtenv dùng nhãn `BLOCKING` cho cả cổng `alternate` và cổng vừa khôi phục còn chờ BPDU. Hàm `refreshDisplay()` giờ hiển thị `WAITING BPDU` khi cổng có `awaitingBpdu=true`. Các cổng đang chặn do vai trò `alternate` tiếp tục hiển thị `BLOCKING`. [Mã hiển thị](src/StpBridge.cc#L216)

| Mốc kiểm tra | Hiển thị trên cả K7-STP và K7-RSTP |
| --- | --- |
| Trước 12 s | Dây b0–b4 đỏ đứt vì đang down. Cổng b3 hướng b4 mang nhãn `BLOCKING p1 -> b4`. |
| Sau các sự kiện ở 12 s, trước khi BPDU tới ở 12,01 s | Dây b0–b4 cam đứt. b0 hiện `WAITING BPDU p1 -> b4` và b4 hiện `WAITING BPDU p1 -> b0`. Nhãn `BLOCKING p1 -> b4` vẫn nằm trên b3. |
| Từ 12,01 s, sau khi hai đầu nhận BPDU | Hai nhãn `WAITING BPDU` biến mất và dây b0–b4 xanh liền. Cổng b4 hướng b1 hiện `BLOCKING p2 -> b1`. |

Đã đối chiếu 15 mốc Qtenv với trace STP, gồm lúc mở dây ở cả hai lượt K7. Cả tám lượt Cmdenv đạt kiểm tra hồi quy và cho cùng số liệu hội tụ, probe như trước khi đổi nhãn. Để quan sát khoảng chờ ngắn khi demo, dùng **Step** quanh mốc 12 s. Phép kiểm tra đồ họa dùng Qtenv kết xuất ngoài màn hình.

Bằng chứng của lượt kiểm tra nằm trong thư mục log cục bộ `out/review-visual/`. [Ảnh K7-STP lúc chờ BPDU](out/review-visual/qtenv-waiting/K7-STP-open-view0.png) · [Ảnh K7-RSTP lúc chờ BPDU](out/review-visual/qtenv-waiting/K7-RSTP-open-view0.png) · [Kết quả Qtenv](out/review-visual/qtenv-waiting/summary.txt) · [Hồi quy Cmdenv](out/review-visual/cmdenv-waiting/summary.txt)

### Probe K7

| Thời điểm phát | K7-STP nhận / phát | K7-RSTP nhận / phát |
| --- | ---: | ---: |
| Trước 12 s | 800/800 | 800/800 |
| Đúng 12 s | 19/20 | 17/20 |
| Các lượt từ 12,1 s còn trong thời gian đổi cây | 400/400 | 0 lượt |
| Từ sau mốc đổi cuối đến trước 23 s | 1780/1780 | 2180/2180 |
| Tổng | 2999/3000 | 2997/3000 |
| Bản sao bị loại do cổng chặn / trùng / giới hạn hop | 3026 / 2 / 0 | 3010 / 3 / 0 |

- Trước sự kiện, probe b0 → b3 `seq=2` đi b0@8 → b1@8,01 → b2@8,02 → b3@8,03. Sau hội tụ, cùng cặp `seq=402` đi b0@18 → b4@18,01 → b3@18,02. Tất cả ID đến đích trong hai cửa sổ ổn định khớp đường trên cây. [Đường đi kiểm tra](out/review-k1-k7/inspect.txt)
- **Mất ở cả hai lượt.** b0 → b4 `seq=163` phát lúc 12 s, khi cổng trực tiếp vẫn blocking, đi b0 → b1 → b4. Bản sao đến b4 lúc 12,02 s sau khi b4 đã chặn hướng b1. [STP](out/review-k1-k7/K7_STP/cmdenv.log#L5644) · [RSTP](out/review-k1-k7/K7_RSTP/cmdenv.log#L5695)
- **Hai ID mất thêm ở rapid.** b0 → b3 `seq=162` đi b0 → b1 → b2 → b3 và tới lúc 12,03 s. STP còn nhận qua b2, còn rapid đã chặn cổng b3 về b2 lúc 12,02 s. Với b4 → b3 `seq=163`, nhánh trực tiếp đến b3 lúc 12,01 s khi cổng b3 về b4 còn blocking ở cả hai lượt. Nhánh qua b1 và b2 đến lúc 12,03 s, cũng bị chặn ở rapid. Bản sao đi b4 → b1 → b0 → b4 bị loại vì quay về nguồn. [STP nhận b0 → b3](out/review-k1-k7/K7_STP/cmdenv.log#L5676) · [RSTP loại b0 → b3](out/review-k1-k7/K7_RSTP/cmdenv.log#L5733) · [Các nhánh probe](out/review-k1-k7/inspect.txt)

~~~text
PROBE_DROP t=12.02 bridge=b4 src=0 dst=4 seq=163 reason=blocked
PROBE_RECV t=12.03 src=0 dst=3 seq=162
PROBE_DROP t=12.03 bridge=b3 src=0 dst=3 seq=162 reason=blocked
PROBE_DROP t=12.03 bridge=b4 src=4 dst=3 seq=163 reason=duplicate
~~~

Cả hai lượt có hai bản sao từ b4 quay lại b4. Ở rapid có thêm bản sao b2 → b3 `seq=162` đi qua b1, b0, b4 rồi đến b3 tại 12,04 s, sau lần nhận thành công qua b2 lúc 12,01 s. Cả ba bản sao trùng của rapid đều bị loại. Các số đếm bản sao bị chặn gồm cả những nhánh flood của ID đã đến đích. Thống kê theo ID và theo đường đi lấy từ [eventlog K7-STP](out/review-k1-k7/K7_STP/K7_STP-%230.elog) và [eventlog K7-RSTP](out/review-k1-k7/K7_RSTP/K7_RSTP-%230.elog).

## Review thuật toán và biên kiểm thử

- **Thứ tự chọn root port.** Vector là `((rootPriority, rootId), advertisedCost + localPortCost, (senderPriority, senderId), senderPort)`. Cổng nhận có chỉ số thấp nhất thắng nếu cả vector hòa, do vòng lặp tăng dần và chỉ thay ứng viên khi so sánh `<`. Chọn designated so vector quảng bá trên cùng link, dùng cost đã tích lũy của bridge cục bộ, không cộng thêm cost của link đó. K2 kiểm chứng ưu tiên root trước ID và hòa cost giữa hai bridge gửi. K6 kiểm chứng thêm hòa cost 3 ở b6. Nhánh hòa đến `senderPort` hoặc chỉ số cổng nhận **Chưa xác minh bằng lượt chạy**, vì các topology trong phạm vi không có liên kết song song từ cùng bridge gửi. [Khai báo vector](src/StpBridge.cc#L21) · [Root port và vai trò cổng](src/StpBridge.cc#L132)
- **Học và tính lại theo cổng.** BPDU hợp lệ mới nhất thay `received` của đúng cổng vào rồi gọi `recompute()`. Có kiểm tra ID, priority, cost, port ID, thời hạn và tràn số khi cộng cost. Cổng alternate vẫn học BPDU, ví dụ K4 b1 nhận từ b0 trên cổng 0 ở 4,01 s trong lúc cổng còn blocking. Cổng down xóa BPDU đã học, không tham gia chọn root port, không phát BPDU và từ chối BPDU đang truyền tới. [Nhận BPDU](src/StpBridge.cc#L272) · [K4](out/review-k1-k7/K4/cmdenv.log#L104) · [K3 tại mốc ngắt](out/review-k1-k7/K3/cmdenv.log#L223)
- **Hết hạn và tính lại.** Cổng giữ mốc nhỏ hơn giữa `validUntil` và thời điểm nhận cộng `maxAge`. Timer lên lịch theo mốc gần nhất trong các hạn và sự kiện link. Khi hết hạn, thông tin bị xóa trước khi so ứng viên. Bridge trung gian mang tiếp hạn đã lưu từ root port, nên vòng quảng bá không làm mới hạn gốc. Lượt K1 với `maxAge=0.5s` ghi b1 mất root b0 và tự làm root tại 0,5 s, rồi học lại b0 ở 2,01 s khi có hello mới. [Timer](src/StpBridge.cc#L124) · [Truyền hạn BPDU](src/StpBridge.cc#L232) · [Trace hết hạn và học lại](out/review-k1-k7/expiry/cmdenv.log#L89)
- **Đường dữ liệu.** Bridge gửi bản sao qua cổng ra forwarding, bỏ cổng vào, và bridge nhận kiểm tra lại trạng thái đầu vào tại lúc đến. Một probe hợp lệ có ID nguồn và số thứ tự. Đích nhận dừng phát tiếp, các bản sao trùng và probe hết hop bị loại. Giai đoạn K7 minh họa hai lần kiểm tra theo hai thời điểm khác nhau trên đường, vì vậy bộ lọc trùng vẫn có việc khi cây đổi. [Mã probe](src/StpBridge.cc#L307)
- **Phạm vi còn thiếu bằng chứng.** Độ tương thích với thiết bị IEEE, phép đo tốc độ của giao thức chuẩn và K8–K10 nằm ngoài đợt thử. Các nhánh từ chối BPDU cấu tạo sai được đọc mã, chưa tiêm gói lỗi để chạy. Trong cấu hình nguyên bản K1–K6, chuyển tiếp dữ liệu chỉ được xác minh qua các lượt bật probe bổ sung như ghi trên.

## Môi trường, lệnh chạy lại và log

Đã đọc `AGENTS.md`, `README.md`, `PLAN.md`, `PARAMETERS.md` và các điểm chạy, NED, C++, BPDU, script kiểm tra liên quan. Ubuntu 26.04 LTS trên WSL2, kernel `6.18.33.2-microsoft-standard-WSL2`, OMNeT++ 6.5.0 build `261029-ccd51e11a4`, `opp_env 0.36.2.261030`, GCC/g++ 15.2.0, GNU Make 4.4.1 và Python 3.14.4. WSL trong sandbox trả `Wsl/E_ACCESSDENIED`. Lệnh chạy WSL ngoài sandbox sau đó thành công. Build dùng bản sao sạch của đầu vào, sinh lại `Bpdu_m.*` và tạo `src/stp_dbg` trong thư mục tạm. Các đầu vào được so byte với repo sau khi chạy. [Phiên bản và lệnh thực tế](out/review-k1-k7/toolchain.log) · [Build](out/review-k1-k7/build.log) · [So sánh đầu vào](out/review-k1-k7/check-tools.log)

Từ Bash trong Ubuntu WSL, chạy lại bằng các lệnh đã dùng. Chạy lại sẽ ghi đè các log cùng tên dưới `out/review-k1-k7/`.

~~~bash
cd ~/omnet-ws/stp
~/opp-venv/bin/opp_env --version > out/review-k1-k7/toolchain.log 2>&1
~/opp-venv/bin/opp_env run omnetpp-6.5.0 -c 'bash out/review-k1-k7/run.sh' >> out/review-k1-k7/toolchain.log 2>&1
~/opp-venv/bin/opp_env run omnetpp-6.5.0 -c 'bash out/review-k1-k7/extra.sh' > out/review-k1-k7/extra.log 2>&1
~/opp-venv/bin/opp_env run omnetpp-6.5.0 -c 'bash out/review-k1-k7/cost.sh' > out/review-k1-k7/cost.log 2>&1
python3 out/review-k1-k7/inspect.py > out/review-k1-k7/inspect.txt
bash out/review-k1-k7/check-tools.sh > out/review-k1-k7/check-tools.log 2>&1
~~~

[Script build và chạy chính](out/review-k1-k7/run.sh) sao chép Makefile và các đầu vào sang thư mục tạm, rồi chạy `make MODE=debug`. Từ `simulations/` của bản sao, lệnh thực tế cho mỗi cấu hình là mẫu dưới đây. `log_dir` là đường dẫn tuyệt đối tới `out/review-k1-k7/<tên-lượt>` trong repo gốc. Hai lượt K7 chính thêm `--record-eventlog=true`. Các lượt `probe-K1` đến `probe-K6` thêm `'--**.probeInterval=0.1s' --record-eventlog=true`. Nhật ký thực thi ghi cả tham số của các lượt chẩn đoán. Các lượt thử selector theo tên cổng đầy đủ trong hai nhật ký đầu bị báo cấu hình không có hiệu lực. Kết luận về cost từng chiều dùng hai lượt selector theo tên bridge trong cost.sh, đã đối chiếu giá trị thực tế trong .sca.

~~~bash
../src/stp_dbg -u Cmdenv -n .:../src -c "$config" \
    --cmdenv-express-mode=false --result-dir="$log_dir" \
    > "$log_dir/cmdenv.log" 2>&1
~~~

| Bằng chứng | Đường dẫn tương đối từ gốc |
| --- | --- |
| Tám lượt chính | [K1](out/review-k1-k7/K1/cmdenv.log), [K2](out/review-k1-k7/K2/cmdenv.log), [K3](out/review-k1-k7/K3/cmdenv.log), [K4](out/review-k1-k7/K4/cmdenv.log), [K5](out/review-k1-k7/K5/cmdenv.log), [K6](out/review-k1-k7/K6/cmdenv.log), [K7_STP](out/review-k1-k7/K7_STP/cmdenv.log), [K7_RSTP](out/review-k1-k7/K7_RSTP/cmdenv.log) |
| Sáu lượt bổ sung dữ liệu | `out/review-k1-k7/probe-K1/` đến `out/review-k1-k7/probe-K6/`, mỗi thư mục có `cmdenv.log`, `.sca` và `.elog` |
| Tái hiện | `out/review-k1-k7/{in-flight,restore-in-flight,restore-in-flight-probe,expiry,isolate-root,cost-b0-out,cost-b1-out,phase-STP,phase-RSTP}/cmdenv.log` |
| Lệnh và kết quả kiểm tra | [toolchain.log](out/review-k1-k7/toolchain.log), [extra.log](out/review-k1-k7/extra.log), [cost.log](out/review-k1-k7/cost.log), [summary.txt](out/review-k1-k7/summary.txt), [inspect.txt](out/review-k1-k7/inspect.txt), [check-tools.log](out/review-k1-k7/check-tools.log) |

Tất cả cấu hình chính trong lượt review ban đầu kết thúc theo giới hạn mô phỏng 25 s. Các lượt chẩn đoán có override được ghi riêng theo tên lượt.
