# bsp_uart

基于 Zephyr Async API 的 UART 驱动封装，使用 DMA 收发 + 空闲超时检测，支持不定长数据接收。

## 功能

- DMA 双缓冲接收，不丢字节
- 空闲超时触发（默认 10ms），支持不定长数据帧
- 环形缓冲区存储接收数据（256 字节）
- 发送信号量保护，防止 DMA 发送重叠
- 支持注册接收回调函数，每个 UartObj 独立

## API

```cpp
// 初始化，rx_timeout 单位 µs，默认 10000（10ms）
int  bsp_uart_init(BspUartObj& obj, const device* dev, uint32_t rx_timeout = 10000);

// 注册接收回调，在 UART_RX_RDY 中断中调用
void bsp_uart_set_rx_callback(BspUartObj& obj, BspUartRxCallback cb, void* arg = nullptr);

// DMA 异步发送，会等上一次发完再发
int  bsp_uart_send(const BspUartObj& obj, const uint8_t* data, uint16_t len);

// 从环形缓冲区读取数据
int  bsp_uart_read(BspUartObj& obj, uint8_t* data, uint16_t len);

// 查询环形缓冲区可读字节数
uint16_t bsp_uart_available(const BspUartObj& obj);
```

## prj.conf

```conf
CONFIG_SERIAL=y
CONFIG_UART_ASYNC_API=y
```

如果同时使用 console（需要另一个 UART），还需要：

```conf
CONFIG_CONSOLE=y
CONFIG_UART_INTERRUPT_DRIVEN=y
```

## 设备树配置

### STM32（以 USART1 + DMA2 为例）

```dts
&usart1 {
    pinctrl-0 = <&usart1_tx_pa9 &usart1_rx_pa10>;
    pinctrl-names = "default";
    current-speed = <115200>;
    status = "okay";

    dmas = <&dma2 7 4 (STM32_DMA_PERIPH_TX | STM32_DMA_PRIORITY_HIGH) STM32_DMA_FIFO_FULL>,
           <&dma2 2 4 (STM32_DMA_PERIPH_RX | STM32_DMA_PRIORITY_HIGH) STM32_DMA_FIFO_FULL>;
    dma-names = "tx", "rx";
};

&dma2 {
    status = "okay";
};
```

DMA 通道参数说明（以 `<&dma2 7 4 ... STM32_DMA_FIFO_FULL>` 为例）：
- `&dma2`：DMA 控制器
- `7`：DMA stream 编号（查 STM32 参考手册 DMA request mapping 表）
- `4`：DMA channel 编号
- flags：`STM32_DMA_PERIPH_TX/RX` 方向 | `STM32_DMA_PRIORITY_HIGH` 优先级
- `STM32_DMA_FIFO_FULL`：启用 FIFO，也可用 `0x00` 关闭（直接模式）

### Renesas RA（以 UART1 / SCI1 为例）

```dts
&uart1 {
    pinctrl-0 = <&sci1_default>;
    pinctrl-names = "default";
    current-speed = <115200>;
    status = "okay";

    interrupts = <12 1>, <13 1>, <14 1>, <22 1>;
    interrupt-names = "rxi", "txi", "tei", "eri";
};
```

注意：RA 系列使用 DTC（Data Transfer Controller）实现 async API，不需要单独配置 DMA 节点，但 IRQ 编号不能与其他外设冲突。

## 使用示例

```cpp
static BspUartObj uart1_obj;

void rx_handler(const uint8_t* data, uint16_t len, void* arg)
{
    auto& obj = *static_cast<BspUartObj*>(arg);
    bsp_uart_send(obj, data, len);  // echo
}

void init()
{
    bsp_uart_init(uart1_obj, DEVICE_DT_GET(DT_NODELABEL(usart1)));
    bsp_uart_set_rx_callback(uart1_obj, rx_handler, &uart1_obj);
}
```
