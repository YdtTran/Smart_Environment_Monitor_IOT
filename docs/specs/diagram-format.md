# Diagram Format Spec

Quy ước áp dụng cho toàn bộ file `.drawio` trong [docs/diagrams/](../diagrams/) (L1 context, L2 container, L3 component, và mọi level bổ sung sau này). Mục tiêu: các diagram nhìn như cùng một hệ thống, thay vì mỗi file một phong cách.

## 1. Font

- **Font family** (áp dụng cho mọi cell có chữ — box, cạnh, nhãn, tiêu đề, legend):
  `JetBrains Mono, monospace, Consolas, Courier New, monospace`
  Đặt qua thuộc tính `fontFamily` trong `style`, không dùng thẻ `<font face="...">` lồng trong `value` (tránh trùng lặp, khó sửa hàng loạt).

- **Kích thước & độ đậm — cố định, không được thay đổi tuỳ file**:
  | Vị trí chữ | fontSize | fontStyle |
  |---|---|---|
  | Chữ trong box/khối (component, container, swimlane) | `20` | `1` (bold) |
  | Chữ trên nhãn cạnh/mũi tên (edge label) | `14` | `1` (bold) |
  | Tiêu đề diagram (VD "CONTEXT DIAGRAM") | `22` | `1` (bold) |
  | Legend title | `14` | `1` (bold) |
  | Legend item label | `13` | `1` (bold) |

  Áp dụng đồng nhất cho **mọi** level, kể cả L3 (component diagram) dù box nhỏ/dày đặc hơn — không có ngoại lệ theo file. Khi 1 box không đủ chỗ chứa chữ ở size chuẩn, resize box đó (xem mục 3) thay vì hạ size xuống riêng cho box/file đó.

  Ngoại lệ duy nhất: box có `value` chứa HTML/`<span>` tự set `font-weight`/`white-space` thủ công (do người chỉnh tay trong Draw.io) — không ép `fontSize` lên cell đó để tránh phá vỡ style đã tự căn chỉnh; ghi rõ trong PR/commit khi gặp trường hợp này.

## 2. Màu sắc theo ngữ nghĩa

Mỗi khối được tô màu theo **vai trò**, không theo sở thích cá nhân hay theo file. Dùng đúng 4 nhóm sau, xuyên suốt mọi level:

| Nhóm | Ý nghĩa | fillColor | strokeColor | Style thêm |
|---|---|---|---|---|
| **External** | Nằm ngoài ranh giới hệ thống/diagram đang vẽ (actor, hệ thống bên thứ 3, node ở level khác nhìn từ ngoài) | `#f5f5f5` | `#666666` | `dashed=1;dashPattern=8 8` (hoặc `4 4` cho legend swatch nhỏ) |
| **System / Container boundary** | Khối trung tâm cấp cao nhất của diagram, hoặc khung swimlane/container bao các khối con | `#e6d9f2` | `#7a4fa3` | — |
| **Hardware / Physical device** | Thiết bị phần cứng vật lý, cảm biến, actuator, node triển khai (STM32, ESP32, Raspberry Pi, DHT11, LDR, potentiometer, Servo, OLED, LCD, LED...) | `#fde3cf` | `#d79b00` | — |
| **Logic / Software block** | Hàm, khối xử lý, logic nội bộ bên trong 1 container (chỉ xuất hiện ở level chi tiết như L3) | `#dae8fc` | `#6c8ebf` | — |

Quy tắc chọn nhóm:
- Nếu một box đại diện cho *cái gì đó chạy/tồn tại độc lập ngoài phạm vi hệ thống đang mô tả* → **External**, dù nó có vẻ "quan trọng" (VD: IOT Platform ThingsBoard trong L2 là external vì nằm ngoài phần cứng nhóm tự triển khai; ESP32 nhìn từ L3-STM32 cũng là external).
- Khung bao (swimlane/container) luôn tô **System/Container boundary**, kể cả khi nó nằm lồng trong 1 khung khác.
- Nếu 1 file không có nhóm nào đó (VD L1, L2 không có "Logic block") thì không cần vẽ legend cho nhóm đó — legend chỉ liệt kê những màu **thực sự dùng** trong file đó.

`fontColor` mặc định để `default` (không hard-code màu chữ), trừ khi cần tương phản đặc biệt.

## 3. Bố cục (layout)

- **Bắt buộc**: mỗi diagram phải có khung `border` (`rounded=1`, `fillColor=none`, `strokeColor=#333333`, `strokeWidth=1`) bao quanh toàn bộ nội dung chính, và 1 cell `title` (text, fontSize 22, bold, cùng fontFamily chuẩn) đặt gần góc trên của border. Không có ngoại lệ theo level — L1, L2, L3 và mọi level sau đều phải có cặp `border`/`title` này.
- Không di chuyển/resize box đang có sẵn khi task chỉ yêu cầu đổi màu hoặc thêm legend — trừ khi có yêu cầu riêng về layout hoặc chữ bị tràn (overflow).
- Khi buộc phải resize để tránh chữ tràn khối: ưu tiên nới rộng box tại chỗ (giữ nguyên vị trí `x/y` nếu có thể), tránh dịch chuyển các box khác.
- Legend đặt vào **vùng trống có sẵn** trên canvas (không đè lên box nào) — góc trên/dưới hoặc cạnh trái/phải của khu vực nội dung chính, không chèn giữa các box đang có quan hệ mũi tên.

## 4. Legend

Mỗi diagram có 1 legend riêng, chỉ liệt kê các nhóm màu **đang thực sự xuất hiện** trong file đó (xem bảng mục 2). Cấu trúc:

- 1 panel nền: `rounded=1;fillColor=#ffffff;strokeColor=#cccccc` bao quanh toàn bộ legend.
- 1 cell tiêu đề: text `"Legend"`, bold.
- Mỗi dòng/nhóm gồm 2 cell:
  - **Swatch**: hình chữ nhật bo góc nhỏ (~18–22px), dùng đúng `fillColor`/`strokeColor`/`dashed` của nhóm màu tương ứng ở mục 2 (không cần chữ bên trong).
  - **Label**: text mô tả ngắn gọn nhóm đó (VD "External system", "Container / swimlane", "Hardware / sensor", "Logic / software block").

Bố cục legend có thể xếp dọc (1 cột) hoặc dạng lưới 2 cột tuỳ khoảng trống còn lại trên canvas — miễn giữ đúng thứ tự cặp swatch–label và không đè lên nội dung khác.

## 5. Ví dụ tham chiếu

- [l01-context-diagram.drawio](../diagrams/l01-context-diagram.drawio) — 3 nhóm màu: External, System boundary, Hardware.
- [l02-container-diagram.drawio](../diagrams/l02-container-diagram.drawio) — 2 nhóm màu: External, Hardware/Container.
- [l03-component-stm32.drawio](../diagrams/l03-component-stm32.drawio) — 4 nhóm màu: External, Container/swimlane, Hardware, Logic block; có `border`/`title` riêng, tách khỏi legend đặt ở phần đầu canvas.
- [l03-component-esp32-gateway.drawio](../diagrams/l03-component-esp32-gateway.drawio) — cùng 4 nhóm màu như trên (task FreeRTOS → Logic block, TFT ST7789 → Hardware, STM32/RPi4/WiFi AP → External, khung ESP32 Gateway → Container).

Khi thêm level mới (L4, hoặc component diagram cho Raspberry Pi), áp dụng lại đúng 4 nhóm màu và cấu trúc legend ở trên để giữ tính nhất quán trong toàn bộ `docs/diagrams/`.
