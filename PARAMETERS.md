# Tham số mô phỏng STP và lý do lựa chọn

Tài liệu này ghi các giá trị dự kiến cho Bước 2 của bài tập lớn ET4230 Đề 05. Xem [PLAN.md](PLAN.md) để biết quy trình kiểm thử, đo lường và bàn giao.

## Cấu hình gốc K1

| Tham số | Giá trị dự kiến | Lý do chọn |
| --- | --- | --- |
| Số switch và liên kết | 5 switch và 6 liên kết hai chiều. Vòng ngoài `b0–b1–b2–b3–b4–b0` và dây chéo `b1–b4`. | Có hai vòng độc lập để quan sát các cổng bị chặn. Sơ đồ đủ nhỏ để đối chiếu vai trò từng cổng bằng tay. |
| Trạng thái liên kết | Tất cả hoạt động lúc bắt đầu. | Tạo mốc đối chứng cho các kịch bản thay đổi cấu trúc hoặc gây sự cố. |
| Bridge ID trong mô hình | `0–4` cho mạng chính, thêm `5–7` ở mạng mở rộng. | ID duy nhất và trùng với số thứ tự switch giúp đọc trace và kiểm tra quy tắc phân xử khi bằng priority. |
| Priority | `32768` cho mọi switch. | Giữ cùng priority để kết quả bầu root dựa trên bridge ID. Theo quy tắc mô hình, `b0` dự kiến thắng vì có ID nhỏ nhất. |
| Path cost | `1` cho mỗi chiều liên kết. | Có thể đối chiếu tổng cost bằng cách đếm số chặng. |
| Chu kỳ BPDU | `2 s`. | Kế thừa mô hình tam giác hiện có và giữ cố định giữa các phép so sánh STP. |
| Thời hạn BPDU | `20 s` mặc định. | Root đặt thời hạn khi phát BPDU. Bridge trung gian giữ thời hạn này để thông tin root cũ không được gia hạn khi đi qua vòng. |
| Độ trễ lan truyền | `10 ms` trên mỗi liên kết. | Kế thừa mô hình hiện có. Dùng cùng giá trị để so sánh các thay đổi về priority, cost và topology. |
| Thời lượng | Dự kiến `25 s`. | Chừa thời gian quan sát khởi tạo, sự cố và cây mới. Kiểm tra lại bằng chạy thử, đặc biệt với hai lượt K7. |
| Cửa sổ xác nhận hội tụ | Ít nhất `3` chu kỳ hello sau lần thay đổi cuối, tương đương `6 s` với chu kỳ `2 s`. | Xác nhận cây duy trì trạng thái ổn định qua nhiều lần trao đổi BPDU. Thời gian hội tụ được tính đến lần thay đổi cuối, sau khi kiểm tra cửa sổ xác nhận. |

Cấu hình General dùng 3 switch và chạy 10 s. K1–K7 đã được kiểm tra với thông số chung trong bảng.

## Tham số thay đổi theo kịch bản

| Kịch bản | Thay đổi | Lý do chọn |
| --- | --- | --- |
| K1 | Cấu hình gốc. | Có đáp án tính tay để đối chiếu. Dự kiến 10 cổng forwarding và 2 cổng blocking. |
| K2 | Hạ priority của `b3` từ `32768` xuống `4096` lúc khởi tạo. | Kiểm tra priority có thể đổi kết quả bầu root dù `b3` có bridge ID lớn hơn `b0`. |
| K3 | Ngắt hai chiều `b0–b1` tại giây `10,5`. | Đây là liên kết đang chuyển tiếp. Sau khi ngắt, `b1` dự kiến đổi đường về root qua `b4`. Thời điểm dự kiến chừa khoảng xác nhận cây ban đầu và quan sát tái hội tụ. |
| K4 | Tăng cost hai chiều của `b0–b1` từ `1` lên `3`. | Đường trực tiếp có cost `3`, lớn hơn tổng cost `2` của đường hai chặng qua `b4`. |
| K5 | Bỏ dây chéo `b1–b4` ngay từ đầu. | So sánh tác động của một liên kết dư thừa trong khi giữ nguyên số switch. |
| K6 | Thêm vòng `b4–b5–b6–b7–b4`, nâng lên 8 switch và 10 liên kết. | Kiểm tra mô hình trên mạng lớn hơn có thêm một vòng. Kịch bản thay đổi đồng thời số switch và cấu trúc đồ thị. |
| K7 | Khởi tạo với `b0–b4` tạm ngắt và khôi phục hai chiều tại giây `12`. Chạy `K7_STP` và `K7_RSTP`. Cả hai gửi probe cho đủ 20 cặp có hướng mỗi `0,1 s` trong khoảng `[8, 23)` s. | Dùng cùng một lần khôi phục để so sánh định tính trình tự BPDU, vai trò cổng và khả năng duy trì kết nối. Giữ cùng thời điểm và thời lượng cho cả hai lượt. |
| K8–K10 | Dùng cùng sự cố `b0–b1` như K3, thay tốc độ phát. | Kiểm tra ảnh hưởng của tải đến mất gói khi đổi đường, đồng thời ghi riêng thời gian hội tụ STP. |

## Tham số tải của K8–K10

| Tham số cố định | Giá trị | Lý do chọn |
| --- | --- | --- |
| Luồng dữ liệu | Một luồng tất định từ `b0` đến `b1`. | Theo dõi được cùng cặp nguồn–đích khi đường trực tiếp bị ngắt và gói phải đi qua `b4`. |
| Tốc độ liên kết | `10 Mb/s` mỗi chiều. | Làm mốc để tính tỷ lệ tải và tạo một mức vượt băng thông. |
| Kích thước gói | `1.000 byte` trong mô hình. | Cố định giữa các lượt để quy đổi trực tiếp từ gói/s sang Mb/s. |
| Hàng đợi | Tối đa `32` gói dữ liệu đang chờ mỗi chiều, loại gói mới khi đầy. | Giới hạn giống nhau giữa ba lượt để quan sát khi tải cao gây đầy hàng đợi. |
| Thời gian phát | Từ `8 s` đến trước `23 s`. | Có gói trước, quanh và sau sự cố. Mô phỏng tiếp đến giây 25 để theo dõi gói cuối. |
| Cửa sổ đo theo lúc phát | `[8, 10)`, `[10, 18)`, `[18, 23)` s. | So sánh ba mức tải trên cùng khoảng thời gian. Dùng trace để xác nhận cây đã hội tụ ở cửa sổ đầu và cuối. |

| Kịch bản | Mức tải | Tốc độ phát | Tỷ lệ so với 10 Mb/s |
| --- | --- | --- | --- |
| K8 | Nhẹ | `250 gói/s = 2 Mb/s` | 20% |
| K9 | Trung bình | `750 gói/s = 6 Mb/s` | 60% |
| K10 | Cao | `1.500 gói/s = 12 Mb/s` | 120% |

Tốc độ dữ liệu bằng số gói phát mỗi giây nhân với `1.000 × 8` bit/gói. Mức 120% chủ động vượt tốc độ danh nghĩa của liên kết để kiểm tra mất gói do hàng đợi đầy.

## Chốt thông số sau chạy thử

- Với K3 và K8–K10, kiểm tra cây ban đầu đã ổn định đủ 6 s trước mốc ngắt. Với K7, kiểm tra cả hai lượt đủ thời gian hội tụ trước và sau khôi phục.
- Nếu cần đổi giá trị, ghi giá trị cũ, giá trị mới và lý do. Cập nhật cấu hình, tài liệu và chạy lại các kịch bản liên quan trước khi so sánh.
