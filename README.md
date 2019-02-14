# ONFD (Open NAND Flash Driver)

[![GitHub release](https://img.shields.io/github/release/JayHeng/ONFD.svg)](https://github.com/JayHeng/ONFD/releases/latest) [![GitHub commits](https://img.shields.io/github/commits-since/JayHeng/ONFD/v1.0.0.svg)](https://github.com/JayHeng/ONFD/compare/v1.0.0...master) [![GitHub license](https://img.shields.io/github/license/JayHeng/ONFD.svg)](https://github.com/JayHeng/ONFD/blob/master/LICENSE)

### 1 ONFD 诞生背景
　　笔者曾在2017年参与过NXP i.MXRT系列芯片的BootROM开发，负责其中Raw NAND设备启动支持，因此研究过一段时间的Raw NAND，也接触过不少NAND芯片，并从零开始写了一整套Raw NAND的driver，后被成功用于i.MXRT的BootROM里。不过当初因i.MXRT芯片项目时间紧急，没有仔细打磨这一套Raw NAND driver，导致Driver与BootROM以及i.MXRT芯片耦合较紧，不具通用性，不能被轻易移植到其他MCU平台，至今仍觉遗憾。  

　　随着Flashless的高性能MCU芯片越来越主流，顺便带火了嵌入式存储设备，Raw NAND作为其中一种性价比较高的存储设备，正在被广泛使用，而其中最常见的便是符合ONFI规范的Raw NAND，笔者当初设计的Raw NAND driver也是基于ONFI规范的。最近在支持客户的项目中发现客户经常遇到各种奇怪的Raw NAND问题，因此如果能有一套易于使用和移植的Raw NAND driver或许能造福大家。  

　　有感于Armink大神的 [SFUD](https://github.com/armink/SFUD) 项目（SFUD是一款使用 JEDEC SFDP 标准的串行 (SPI) Flash 通用驱动库），笔者下定决心重构当初写的Raw NAND driver，做出一款符合 ONFI 标准的并行 NAND Flash 通用驱动库，并将其命名为 ONFD（Open NAND Flash Driver）。  

### 2 ONFD 是什么
　　[ONFD](https://github.com/JayHeng/ONFD) 是一款开源的并行 NAND Flash 通用驱动库。  

### 3 ONFD 如何使用
#### 3.1 计划支持 Flash  

<table><tbody>
    <tr>
        <th>型号</th>
        <th>厂商</th>
        <th>容量</th>
        <th>IO宽度</th>
        <th>运行电压</th>
        <th>最高速度</th>
        <th>ONFI标准</th>
        <th>备注</th>
    </tr>
    <tr>
        <td>MT29F4G08ABBDA</td>
        <td>Micron</td>
        <td>4Gb</td>
        <td>8-bit</td>
        <td>1.8V</td>
        <td>40MHz</td>
        <td>支持，1.0</td>
        <td></td>
    </tr>
    <tr>
        <td>MT29F16G08ABACA</td>
        <td>Micron</td>
        <td>16Gb</td>
        <td>8-bit</td>
        <td>3.3V</td>
        <td>50MHz</td>
        <td>支持，1.0</td>
        <td></td>
    </tr>
    <tr>
        <td>MX30LF4GE8AB</td>
        <td>Macronix</td>
        <td>4Gb</td>
        <td>8-bit</td>
        <td>3.3V</td>
        <td>50MHz</td>
        <td>支持，1.0</td>
        <td></td>
    </tr>
    <tr>
        <td>W29N04GVxxAF</td>
        <td>Winbond</td>
        <td>4Gb</td>
        <td>8-bit</td>
        <td>3.3V</td>
        <td>40MHz</td>
        <td>支持</td>
        <td></td>
    </tr>
</table>


