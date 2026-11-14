# K1 — Tính tay cây STP và dòng thời gian BPDU

## 1. Cấu hình mạng

K1 có 5 switch, một vòng ngoài `b0–b1–b2–b3–b4–b0` và dây chéo `b1–b4`. Các switch có priority `32768`, bridge ID từ `0` đến `4`. Mỗi chiều liên kết có path cost `1` và độ trễ lan truyền 10 ms. Chu kỳ hello là 2 s.

## 2. Tính tay cây chuyển tiếp

**Bầu root.** So sánh cặp (priority, bridge ID). Vì priority bằng nhau, `b0` thắng nhờ ID nhỏ nhất. Root path cost của `b0` là 0.

**Chọn root port.** Mỗi switch chọn đường có tổng cost thấp nhất về root. Cost mỗi chặng là 1, nên có thể đếm số chặng. Cổng ở chặng đầu là root port.

| Switch | Đường về root | Tổng cost | Root port hướng về |
| --- | --- | ---: | --- |
| `b0` | Chính nó | 0 | — |
| `b1` | `b1 → b0` | 1 | `b0` |
| `b2` | `b2 → b1 → b0` | 2 | `b1` |
| `b3` | `b3 → b4 → b0` | 2 | `b4` |
| `b4` | `b4 → b0` | 1 | `b0` |

Ví dụ, `b3` đi qua `b4` tốn 2, còn đi qua `b2 → b1 → b0` tốn 3. Vì vậy `b3` chọn cổng hướng `b4`.

**Chọn designated port.** Khi hai đầu cùng chọn `b0` làm root, so sánh root path cost của hai switch. Đầu có cost thấp hơn thắng. Nếu cost bằng nhau, bridge ID nhỏ hơn thắng vì priority bằng nhau.

| Liên kết | Đầu designated | Đầu còn lại |
| --- | --- | --- |
| `b0–b1` | `b0` (cost 0) | `b1` — root port |
| `b0–b4` | `b0` (cost 0) | `b4` — root port |
| `b1–b2` | `b1` (cost 1) | `b2` — root port |
| `b3–b4` | `b4` (cost 1) | `b3` — root port |
| `b2–b3` | `b2` (hòa cost 2, thắng theo ID) | `b3` — alternate, blocking |
| `b1–b4` | `b1` (hòa cost 1, thắng theo ID) | `b4` — alternate, blocking |

Có **6 designated port + 4 root port = 10 cổng forwarding**, cùng **2 cổng blocking** và **0 cổng down**. Bốn liên kết có cả hai đầu forwarding tạo thành cây nối đủ 5 switch.

```text
b2 ── b1 ── b0 ── b4 ── b3
```

## 3. BPDU được gửi từ đâu, đến đâu?

BPDU (Bridge Protocol Data Unit) là thông điệp điều khiển STP. Trong mô hình này, một BPDU mang root mà switch gửi đang chọn, priority của root, cost từ switch gửi về root, ID và priority của switch gửi, cổng gửi và thời điểm hết hiệu lực. Switch nhận cộng cost của cổng nhận vào cost được quảng bá để xét đường đi qua hàng xóm. [Định nghĩa BPDU](src/Bpdu.msg#L15)

Trong K1, mỗi switch tạo một BPDU riêng cho từng cổng liên kết đang hoạt động ở các mốc **0, 2, 4, …, 24 s**. BPDU đi đúng một chặng và đến sau **0,01 s**. Switch hàng xóm dùng thông tin đã học để tạo BPDU của chính nó ở kỳ hello tiếp theo. [Mã gửi và hẹn kỳ hello](src/StpBridge.cc#L232) · [Topology và tham số liên kết](src/StpNet.ned#L55)

| Switch phát | Switch nhận trực tiếp trong mỗi đợt |
| --- | --- |
| `b0` | `b1`, `b4` |
| `b1` | `b0`, `b2`, `b4` |
| `b2` | `b1`, `b3` |
| `b3` | `b2`, `b4` |
| `b4` | `b0`, `b1`, `b3` |

Tổng cộng **12 BPDU mỗi đợt**, tương ứng với 6 liên kết hai chiều. Cổng blocking vẫn trao đổi BPDU trong mô hình K1.

## 4. Tính tay thời gian hội tụ

Lúc 0 s, mỗi switch tự nhận mình là root và quảng bá cost 0. Bảng dưới đây theo dõi các BPDU quyết định thời điểm hội tụ. Cột cost ghi giá trị **do switch gửi quảng bá**.

| Phát (s) | Đến (s) | Nguồn → đích | Root, cost quảng bá | Tác động ở đầu nhận |
| ---: | ---: | --- | --- | --- |
| 0,00 | 0,01 | `b0 → b1`, `b0 → b4` | `b0`, 0 | `b1` và `b4` chọn root `b0`, cost 1. |
| 0,00 | 0,01 | `b1 → b2` | `b1`, 0 | `b2` tạm chọn root `b1`, cost 1. |
| 0,00 | 0,01 | `b2 → b3` | `b2`, 0 | `b3` tạm chọn root `b2`, cost 1. |
| 2,00 | 2,01 | `b1 → b2` | `b0`, 1 | `b2` chọn root `b0`, cost 2. |
| 2,00 | 2,01 | `b4 → b3` | `b0`, 1 | `b3` chọn root `b0`, cost 2, qua `b4`. |
| 2,00 | 2,01 | `b1 → b4` | `b0`, 1 | Hai đầu hòa cost 1. `b1` thắng theo ID, `b4` chặn cổng hướng `b1`. |
| 2,00 | 2,01 | `b2 → b3` | `b1`, 1 | Khi phát ở 2,00 s, `b2` vẫn đang chọn root `b1`. |
| 4,00 | 4,01 | `b2 → b3` | `b0`, 2 | Hai đầu hòa cost 2. `b2` thắng theo ID, `b3` chặn cổng hướng `b2`. |

Ở **2,01 s**, cả 5 switch đã chọn `b0` nhưng BPDU của `b2` gửi lúc 2,00 s vẫn chứa thông tin cũ. Đến **4,01 s**, `b3` nhận BPDU mới của `b2`. Đường về root qua `b2` có cost `2 + 1 = 3`, nên root port của `b3` vẫn hướng `b4`. Khi xét riêng quyền designated trên liên kết `b2–b3`, cả hai switch có root path cost 2 và `b2` có ID nhỏ hơn. Cổng hướng `b2` của `b3` chuyển sang blocking. Đây là thay đổi cuối cùng.

**Thời gian hội tụ từ lúc khởi tạo = 2 × 2 s + 0,01 s = 4,01 s.** Ba chu kỳ hello tiếp theo là cửa sổ xác nhận ổn định, từ 4,01 s đến 10,01 s. Log ghi trạng thái cuối lúc 4,01 s và mô phỏng chạy đến 25 s. [Log K1 tại thời điểm đổi cổng cuối](out/fix-k1-k7/K1/cmdenv.log#L124)

## 5. Đoạn trình bày với thầy

> K1 có cùng priority và cost mỗi chặng, nên `b0` là root vì có ID nhỏ nhất. Ở giây 0, `b0` gửi BPDU cho `b1` và `b4`, hai switch nhận sau 10 ms. Ở giây 2, `b1` và `b4` phát BPDU mang thông tin root đã học, nên `b2` và `b3` chọn được đường về `b0` ở 2,01 giây. `b2` chỉ quảng bá thông tin mới vào kỳ hello ở giây 4. BPDU đó tới `b3` ở 4,01 giây và làm `b3` chặn cổng hướng `b2`. Đây là lần đổi trạng thái cuối, nên thời gian hội tụ là 4,01 giây. Cây cuối có 4 liên kết chuyển tiếp, 10 cổng forwarding và 2 cổng blocking.
