# Kế hoạch bài tập lớn ET4230 Đề 05

## 1. Mục tiêu và phạm vi

Mô phỏng cách Spanning Tree Protocol (STP) tạo cây chuyển tiếp trên mạng switch có đường dự phòng. Dùng OMNeT++ và C++ để quan sát trao đổi BPDU, bầu root bridge, chọn root port và designated port, chặn đường dư thừa và tái hội tụ sau sự cố. Đánh giá bằng trạng thái từng cổng, thời gian hội tụ và khả năng chuyển tiếp gói thăm dò.

Nguồn yêu cầu là đề bài ET4230 cập nhật tháng 10/2026, mục II–VIII và phần riêng của Đề 05. Đề yêu cầu tối thiểu 3 kịch bản, 2 tham số có ý nghĩa và thử thay root hoặc làm hỏng liên kết. Kế hoạch có 10 kịch bản, gồm 6 kịch bản STP, 1 kịch bản mở rộng so sánh STP với RSTP và 3 kịch bản tải nhẹ, trung bình và cao khi có sự cố. Nhóm 5 người điền tên, mã học kỳ, số nhóm và mốc nộp sau khi lớp công bố. Trình giảng viên phê duyệt đề, phạm vi và phân công trước khi triển khai.

Mô hình dùng một cây chuyển tiếp, liên kết điểm–điểm và các trạng thái forwarding, blocking, down. Báo cáo mô tả quy tắc BPDU, timer và chuyển trạng thái được mô phỏng, cùng các giản lược về định dạng khung Ethernet, bảng MAC và các giai đoạn chuyển trạng thái theo chuẩn. Nhóm đưa phần mở rộng tùy chọn RSTP của đề vào kịch bản K7.

## 2. Điểm xuất phát và mô hình đề xuất

- [StpNet.ned](src/StpNet.ned) có ba bridge `b0`, `b1`, `b2` nối tam giác, mỗi liên kết trễ 10 ms. Giữ lại để kiểm thử nhanh với kết quả có thể tính tay.
- [StpBridge.cc](src/StpBridge.cc) gửi BPDU định kỳ và lưu tuyến về root tốt nhất đã thấy. Cần lưu thông tin theo từng cổng và tính lại khi dữ liệu thay đổi hoặc mất hiệu lực. Cần bổ sung designated port, trạng thái cổng, xử lý sự cố và đo lường.
- [Bpdu.msg](src/Bpdu.msg) hiện mang root ID, path cost và bridge ID gửi. Sửa định nghĩa thông điệp khi cần thêm priority hoặc thông tin cổng, rồi để bộ build sinh lại mã thông điệp.
- [omnetpp.ini](simulations/omnetpp.ini) hiện có cấu hình mặc định 10 giây. Giữ một điểm chạy chính trong `simulations/`. Kiểm tra script [run](simulations/run) khi chuẩn hóa lệnh build và chạy vì script gọi `src/stp` còn workspace hiện có `src/stp_dbg`.

**Mạng chính 5 switch.** Vòng ngoài `b0–b1–b2–b3–b4–b0` và dây chéo `b1–b4`. Sáu liên kết tạo hai vòng độc lập. Mỗi liên kết có hai cổng, một ở mỗi đầu.

**Mạng mở rộng 8 switch.** Giữ nguyên mạng chính và thêm vòng `b4–b5–b6–b7–b4`. Có 10 liên kết và ba vòng độc lập. Dùng lại cùng mô-đun bridge, BPDU và phương pháp đo.

## 3. Kịch bản và kết quả kỳ vọng

Xem [PARAMETERS.md](PARAMETERS.md) để tra giá trị khởi tạo, tham số thay đổi theo từng kịch bản và lý do chọn.

Thông số dự kiến dùng chung gồm hello interval 2 s, trễ 10 ms mỗi liên kết, cost mặc định 1, priority mặc định 32768 và ID của `b0` đến `b7` lần lượt từ 0 đến 7. Dự kiến chạy 25 s mỗi kịch bản. Cấu hình và thời gian đo được chốt trước đợt thu dữ liệu chính thức. Nếu phải điều chỉnh sau chạy thử, ghi lý do và chạy lại toàn bộ kịch bản liên quan.

| Mã | Thay đổi so với kịch bản gốc | Kết quả dự kiến sau hội tụ |
| --- | --- | --- |
| K1 – Mạng gốc | Mạng 5 switch đủ 6 liên kết, mọi priority và cost mặc định | `b0` là root. `b3` chặn phía nối `b2`, `b4` chặn phía nối `b1`. 10 forwarding, 2 blocking. |
| K2 – Đổi root | Giảm priority của `b3` xuống 4096 ngay trong cấu hình khởi tạo | `b3` là root. `b1` chọn đường qua `b2` và chặn các cổng về `b0`, `b4`. 10 forwarding, 2 blocking. |
| K3 – Hỏng liên kết | Ngắt cả hai chiều của `b0–b1` tại 10,5 s, sau khi mạng gốc đã ổn định | Root vẫn là `b0`. `b1` đổi root port sang phía `b4`. Sau tái hội tụ có 9 forwarding, 1 blocking, 2 down. |
| K4 – Đổi path cost | Tăng cost của `b0–b1` lên 3 ở cả hai chiều, giữ nguyên topology | Root vẫn là `b0`. `b1` ưu tiên đường hai hop qua `b4` với cost 2. 10 forwarding, 2 blocking. |
| K5 – Bỏ dây chéo | Bỏ `b1–b4` từ đầu, giữ nguyên 5 switch | `b0` là root. Chỉ còn cổng của `b3` hướng `b2` ở blocking. 9 forwarding, 1 blocking. |
| K6 – Mạng 8 switch | Thêm vòng `b4–b5–b6–b7–b4` | `b0` là root. 17 forwarding, 3 blocking. Kịch bản kiểm tra mô hình trên đồ thị lớn hơn có thêm vòng. |

K1–K2 khảo sát priority, K1–K3 khảo sát sự cố, K1–K4 khảo sát cost và K1–K5 khảo sát liên kết dư thừa. K6 thay đổi đồng thời số switch và cấu trúc đồ thị, vì vậy kết luận chỉ áp dụng cho topology thử nghiệm. Các số trong bảng là dự đoán để kiểm thử. Ghi số liệu thực nghiệm từ các lượt chạy và giải thích mọi sai khác.

### K7 – So sánh STP và RSTP khi khôi phục liên kết

Dùng mạng 5 switch với liên kết `b0–b4` tạm ngắt từ đầu. Dự kiến khôi phục cả hai chiều tại giây 12. Chạy thử để xác nhận cả hai mô hình hội tụ trước khi khôi phục và có đủ thời gian quan sát sau đó. Nếu cần điều chỉnh mốc khôi phục hoặc thời lượng mô phỏng, áp dụng cùng giá trị cho cả hai lượt chính thức. Chạy một lượt STP và một lượt RSTP, giữ nguyên sơ đồ, priority, cost, độ trễ và lịch gửi probe. Kỳ vọng `b0` giữ vai trò root và `b4` đổi root port từ phía `b1` sang đường trực tiếp về `b0`.

Đặt hai trace cạnh nhau để so sánh định tính thứ tự BPDU, các vai trò và trạng thái cổng thay đổi, lúc đường trực tiếp bắt đầu chuyển tiếp và khả năng duy trì kết nối trong quá trình đổi đường. Ghi rõ timer, cơ chế chuyển trạng thái và phần giản lược của từng mô hình khi giải thích khác biệt.

### Tải nhẹ, trung bình và cao khi hỏng liên kết

K8–K10 dùng mạng 5 switch và ngắt cùng liên kết `b0–b1` ở 10,5 s như K3. Giữ nguyên priority, cost, lịch bắt đầu/kết thúc phát, thời điểm ngắt link, độ trễ lan truyền, kích thước gói, tốc độ đường truyền và dung lượng hàng đợi. Biến khảo sát là tốc độ phát của một luồng tất định từ `b0` đến `b1`. Phát gói đều từ 8 s đến trước 23 s, mỗi gói 1.000 byte theo kích thước mô hình. Mỗi chiều của mỗi liên kết có tốc độ 10 Mb/s và hàng đợi 32 gói dữ liệu đang chờ, loại gói mới khi đầy.

| Mã | Mức tải | Tốc độ phát | Tải dữ liệu chào vào so với tốc độ một chiều của liên kết |
| --- | --- | --- | --- |
| K8 | Nhẹ | 250 gói/s | 2 Mb/s (20%) |
| K9 | Trung bình | 750 gói/s | 6 Mb/s (60%) |
| K10 | Cao | 1.500 gói/s | 12 Mb/s (120%) |

Tải cao cố ý vượt năng lực danh nghĩa của liên kết để quan sát hàng đợi và mất gói. Cả ba kịch bản dự kiến về cùng cây chuyển tiếp với K3 sau khi hội tụ. Đo riêng thời gian hội tụ STP và tỷ lệ gói đến đích để phân biệt tác động của thay đổi đường đi với tác động của quá tải.

## 4. Hiện thực và kiểm thử

1. **Build và chạy mô hình ba switch.** Ghi phiên bản OMNeT++, hệ điều hành, trình biên dịch và lệnh chạy Cmdenv đã kiểm tra. Lưu log dùng làm mốc. Mô hình hiện tại được đọc từ mã nguồn, chưa xác minh bằng lượt chạy mới.
2. **Hoàn thiện thuật toán phân tán.** Mỗi bridge tự quyết định từ BPDU nhận trên các cổng. Dùng cặp `(priority, bridgeId)` để xếp hạng bridge. Khi chọn đường về root, so sánh lần lượt định danh root, tổng cost, định danh bridge gửi và định danh cổng để xử lý hòa. Lưu BPDU hợp lệ gần nhất theo cổng và tính lại từ các ứng viên còn hiệu lực. So sánh thông tin hai đầu mỗi liên kết để chọn designated port. Cho root port và designated port chuyển tiếp dữ liệu khi vai trò đã được xác lập. Cổng blocking vẫn nhận BPDU.
3. **Thêm sự cố và gói thăm dò.** Lập lịch ngắt hai chiều, cập nhật trạng thái hai đầu cổng và loại thông tin học qua cổng hỏng. Kiểm tra BPDU và gói đang truyền quanh thời điểm sự cố. Chuyển tiếp probe chỉ qua cổng forwarding ở cả hai đầu, loại cổng vào khi phát tiếp và gắn ID để phát hiện gói lặp. Dùng giới hạn hop để kết thúc lượt chạy nếu có vòng trong giai đoạn chuyển tiếp, đồng thời ghi sự kiện chạm giới hạn.
4. **Thêm RSTP và sự kiện khôi phục liên kết cho K7.** Sau khi kiểm tra mô hình STP cơ bản, bổ sung chế độ RSTP giản lược. Ghi trace BPDU và thay đổi vai trò, trạng thái cổng để giải thích khác biệt giữa hai lượt chạy.
5. **Thêm băng thông và hàng đợi cho K8–K10.** Mô phỏng thời gian truyền theo kích thước gói và tốc độ liên kết, dùng cùng quy tắc cổng forwarding cho luồng dữ liệu. Cho BPDU ưu tiên khi đường truyền rảnh và để gói đang truyền hoàn tất. Ghi nguyên nhân mất gói, gồm hàng đợi đầy, cổng chuyển trạng thái và liên kết hỏng.
6. **Kiểm thử bằng kết quả tính tay.** Hai switch kiểm tra chọn root và kết nối. Tam giác kiểm tra một cổng blocking và hai liên kết chuyển tiếp được ở cả hai đầu. Với K1–K10, đối chiếu root, root port, designated port, số cổng từng trạng thái và cấu trúc cây. Kiểm tra cổng down không mang dữ liệu. K3 và K8–K10 phải chuyển đường qua `b4`, còn ở K7, `b4` phải quay về đường trực tiếp khi liên kết hoạt động lại. Thử riêng một liên kết để xác nhận tốc độ phát, thời gian truyền và giới hạn hàng đợi trước khi đo trên mạng 5 switch. Nếu có gói lặp hoặc mất kết nối trong lúc chuyển vai trò, ghi rõ thời điểm và phân tích từ trace.
7. **Chạy lại từ bản sao sạch.** Một thành viên dùng README để build và chạy các cấu hình bằng chính tệp dự kiến nộp. So sánh kết quả định lượng, kiểm tra lỗi mô phỏng và xác minh script dựng lại được các biểu đồ.

## 5. Đo lường và phân tích

| Chỉ tiêu | Cách đo và tiêu chí đối chiếu |
| --- | --- |
| Cổng forwarding, blocking, down | Đếm từng đầu cổng ở ảnh chụp trạng thái sau hội tụ. Đếm riêng liên kết sử dụng được khi cả hai đầu forwarding. Sau hội tụ, đồ thị chuyển tiếp phải liên thông và có đúng `N−1` liên kết với `N` switch còn nối được trong các kịch bản đã chọn. |
| Thời gian hội tụ (s) | Từ lúc khởi tạo hoặc thời điểm sự cố đến lần đổi cuối cùng của root, cost, root port, vai trò hay trạng thái cổng. Xác nhận cấu hình đúng và ổn định trong ít nhất 3 chu kỳ hello sau lần đổi cuối, đồng thời kiểm tra phần log còn lại. Ghi riêng hội tụ ban đầu và tái hội tụ ở K3. Nếu hết thời gian quan sát mà chưa xác nhận, ghi rõ trạng thái đó. |
| Khả năng kết nối (%) | Gửi probe cho mọi cặp nguồn–đích có thứ tự. Tỷ lệ kết nối là số cặp có probe đến đích chia tổng số cặp đã thử, nhân 100%. Kỳ vọng sau hội tụ là 20/20 cặp với mạng 5 switch và 56/56 với mạng 8 switch. K3 đo trước khi ngắt và sau khi hội tụ lại. Lưu thêm số probe lặp và thời điểm gửi/nhận để kiểm tra. |
| So sánh định tính ở K7 | Đặt trace STP và RSTP trên cùng trục thời gian, đánh dấu lúc `b0–b4` hoạt động lại, các lần đổi vai trò/trạng thái và probe nhận được. |
| Tỷ lệ gói đến đích và số gói rớt ở K8–K10 | Phân nhóm theo thời điểm phát với ba cửa sổ dự kiến `[8, 10)`, `[10, 18)`, `[18, 23)` s. Đếm mỗi ID đến đích một lần và chia cho số gói phát trong cùng cửa sổ. Chốt kết quả khi kết thúc mô phỏng ở 25 s. Đối chiếu trace để xác nhận cửa sổ đầu và cuối đã hội tụ. Ghi rõ nếu chưa xác nhận được và thống kê riêng số lần loại gói vì đầy hàng đợi hoặc sự cố cổng/liên kết. |

Lưu trace có mã kịch bản, thời gian mô phỏng, bridge/cổng, root, cost, vai trò, trạng thái và sự kiện link/gói. Lưu CSV tổng hợp kèm tham số, phiên bản công cụ, mã lượt chạy và quy tắc tính. Dùng script đọc dữ liệu để vẽ cột số cổng theo kịch bản, biểu đồ thời gian hội tụ, dòng thời gian quanh sự cố K3 và tỷ lệ gói đến đích ở ba mức tải. Mỗi hình ghi tiêu đề, đơn vị, chú giải và điều kiện đo.

Mô hình dự kiến tất định. Chạy lại để kiểm tra khả năng tái lập. Nếu thêm yếu tố ngẫu nhiên, dùng ít nhất 5 seed theo khuyến nghị của đề, ghi seed và trình bày trung bình cùng độ phân tán. Trong báo cáo, phân biệt số liệu quan sát, kết luận suy ra từ mô hình và giới hạn khi áp dụng cho mạng thực.

## 6. Mốc thực hiện và phân công

Điền ngày cụ thể sau khi có lịch lớp. Lùi từ hạn nộp chính thức để dành riêng một mốc chạy lại, đóng gói và diễn tập bảo vệ. Cập nhật hạn, trạng thái và đường dẫn minh chứng ở mỗi mốc để nhóm dùng bảng này theo dõi tiến độ.

| Mốc | Điều kiện hoàn thành | Phụ trách đề xuất | Hạn và trạng thái |
| --- | --- | --- | --- |
| M1 – Đề xuất và môi trường | Giảng viên duyệt phạm vi, sơ đồ, baseline và phân công. Nhóm chạy được mô hình ba switch. | TV1 điều phối, TV2 xác minh build | Chưa chốt |
| M2 – Cây chuyển tiếp | BPDU, bầu root, root port, designated port và blocking chạy đúng ở tam giác và K1. | TV2 thuật toán, TV3 topology và kiểm thử | Chưa chốt |
| M3 – Sự cố và đo lường | K2–K10 chạy được. Trace đủ để tính chỉ tiêu STP, so sánh hai lượt K7 và đối chiếu ba mức tải. | TV3 kịch bản, TV4 đo lường, TV2 hỗ trợ | Chưa chốt |
| M4 – Thực nghiệm và báo cáo | Có dữ liệu thô, CSV, script biểu đồ và bản báo cáo có đối chiếu dự đoán với thực nghiệm. | TV4 dữ liệu, TV1 tổng hợp, cả nhóm viết phần mình | Chưa chốt |
| M5 – Nộp và bảo vệ | Chạy lại từ bản nộp, kiểm tra ZIP, hoàn thành slide/demo và diễn tập phần cá nhân. | TV5 đóng gói và demo, cả nhóm bảo vệ | Chưa chốt |

TV1–TV5 là vị trí để nhóm điền tên thật. Mỗi thành viên ghi nhật ký theo ngày, việc đã làm, tệp hoặc bằng chứng, cách kiểm thử và kết quả. Mỗi người chuẩn bị giải thích quyết định của mình và thao tác demo. Rubric gồm 30% báo cáo, 40% mô phỏng/chương trình và 30% bảo vệ cá nhân.

## 7. Cấu trúc và sản phẩm bàn giao

Mục III của đề khuyến nghị các thư mục `report/`, `src/`, `configs/`, `results/`, `plots/`, `demo/` và tệp `README.md`. Nhóm dùng bố cục dưới đây cho bản ZIP và giữ `simulations/` làm điểm chạy của dự án OMNeT++ hiện tại. Tên các tệp bên dưới là tên dự kiến.

```text
ET4230_HK[ky]_N[so-nhom]_D05.zip
├── report/
│   ├── bao-cao.pdf
│   ├── bang-phan-cong.md
│   └── nhat-ky-dong-gop.md
├── src/                     Mã C++, mô hình NED, định nghĩa thông điệp và tệp build
├── configs/
│   └── stp.ini              Tham số và cấu hình chạy K1–K10
├── simulations/
│   ├── omnetpp.ini          Điểm chạy OMNeT++, tham chiếu cấu hình trong configs/
│   ├── package.ned
│   └── run                  Script chạy mô phỏng
├── results/
│   ├── raw/                 Log và trace theo kịch bản, mã lượt chạy
│   └── summary.csv          Kết quả tổng hợp và tham số đo
├── plots/
│   ├── plot_results.py      Script dựng lại biểu đồ từ results/
│   └── figures/             Biểu đồ dùng trong báo cáo
├── demo/
│   ├── bao-ve.pptx
│   └── demo.mp4             Hoặc thư mục minh chứng gồm ảnh, log hay trace
├── Makefile
└── README.md
```

Mỗi kết quả đo phải tra được đơn vị, giá trị tham số, phiên bản và cấu hình môi trường mô phỏng, cách tính chỉ tiêu và lượt chạy tạo ra dữ liệu. README chỉ cách chạy lại từ thư mục vừa giải nén và cách dùng script tạo biểu đồ. Chỉ đóng gói mã, cấu hình, dữ liệu và minh chứng cần thiết. Tệp biên dịch, máy ảo và dữ liệu quá lớn được loại khỏi bản nộp nếu có thể tái tạo.

## 8. Checklist nộp bài

- [ ] `report/` có một báo cáo PDF. Khuyến nghị 15–25 trang ngoài phụ lục. Nội dung gồm mục tiêu, lý thuyết và nguồn, mô hình và giả định, thiết kế thí nghiệm, kết quả, phân tích, hạn chế, kết luận và tài liệu tham khảo.
- [ ] `src/`, `simulations/` và `configs/` có đủ tệp để build và chạy. README chỉ rõ cách chạy từng cấu hình từ điểm chạy chính.
- [ ] `results/` có dữ liệu gốc và kết quả tổng hợp. `plots/` có biểu đồ và script tái tạo từ dữ liệu đã nộp.
- [ ] `README.md` ghi phiên bản, cài đặt, lệnh build, cách chạy từng kịch bản, xuất dữ liệu và dựng biểu đồ. Có một bài chạy ngắn để dùng khi bảo vệ.
- [ ] `demo/` có video 3–5 phút hoặc minh chứng tương đương, cùng slide bảo vệ ngắn gọn. `report/` có bảng phân công và nhật ký đóng góp của đủ 5 thành viên.
- [ ] Trích dẫn tài liệu, mã nguồn, hình và thuật toán kế thừa. Khai báo phần có AI hỗ trợ và ghi cách nhóm kiểm chứng. Dữ liệu và kết quả lấy từ lượt chạy thực tế.
- [ ] Kiểm tra giải nén, chạy lại, bỏ tệp build và dữ liệu không cần thiết. Kiểm tra tệp nộp không chứa dữ liệu nhạy cảm, mật khẩu hoặc khóa API.
- [ ] Đặt tên `ET4230_HK[ky]_N[so-nhom]_D05.zip` bằng mã kỳ và số nhóm thực tế. Nộp bản mềm trước khi kết thúc học kỳ 3 tuần theo lịch và kênh giảng viên công bố. Chuẩn bị bản in nếu giảng viên yêu cầu.

Tài liệu đối chiếu khi triển khai. Ghi đúng phiên bản đã sử dụng trong báo cáo. [IEEE 802.1Q-2022 Bridges and Bridged Networks](https://1.ieee802.org/maintenance/p802-1q-rev/), [OMNeT++ Simulation Manual](https://doc.omnetpp.org/omnetpp/SimulationManual.pdf).
