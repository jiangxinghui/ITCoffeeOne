currently, working at itcoffeeone-lite branch.


# ITCoffee One

## Project Introduction

ITCoffee One is a DIY coffee machine project, based on the modification of the De'Longhi ECO310 coffee machine. It aims to provide users with a unique experience of making homemade coffee. Through the modification of the ECO310, users can easily create high-quality personalized coffee at home.



currently two branchs included:
1. itcoffee one lite: arduino nano has very limited RAM and Flash. so , lite version contains  limited function, just some basic functionality that control eco310.
   video :
   
3. itcoffee one：
   stm32 based.full functionality profile included.
    

This project is inspired by Gaggiuino. and more
1. lcd hmi removed. because gaggiuino program couples with lcd deeply, not easy to change to other hmi devices
2. modbus protocol included. no matter industry control HMI , or ipad, android,  modbus protocol is the only requirement.
3. software modular. heater , pump , will be separated, and easy to expand when multiple boiler included.
4. 

## Usage Instructions

This project is in its early stages.

### Plan


arduino  based: a lot of arduino diy module available.
freertos: rtos included, and message interchange between boiler controller, pump controller and io controller. 
Modbus:  widely used protocol, and IO available easy.  and UI can be IOS, android, etc mobile app, also Industry control HMI.

Now, development started, and vscode+arduino freertos+modbus, is ready.


1. Heating control section(developing)
   temperature read added. and temperature value is stored to Modbus Holding registers. 
   

   video:
   看看【IT咖啡的作品】用arduino控制咖啡机的加热  https://v.douyin.com/iFGxaUU5/ S@Y.MJ 03/31 EHV:/
   
   
3. Pressure control section(future)

4. IO(future)
5.  PCB board(futrue)
6. Android iOS APP(developing)
  maui bluetooth app developing.

  currently working on ITCoffee-One-Lite branch.


## Contribution

We welcome and appreciate any form of contribution. If you'd like to contribute to the project, please refer to the [Contribution Guidelines], submit issues, propose features, or contribute code.

## License

ITCoffee One project is licensed under the MIT License.

```
MIT License

Copyright (c) 2024 Michael Jiang

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## Acknowledgments

Special thanks to the [Gaggiuino](https://github.com/Zer0-bit/gaggiuino) project and the De'Longhi ECO310 coffee machine. Our project has been inspired and influenced during the design and implementation process.

## DIY Records and Discussions

We encourage everyone to record issues, thoughts, and discussions related to the DIY process in the [Issues](https://github.com/jiangxinghui/itcoffee-one/issues) section. Share your experiences, ask questions, and engage with other community members. Let's work together to create the perfect ITCoffee One!






# ITCoffee One

## 项目简介

ITCoffee One 是一个 DIY 咖啡机项目，基于德龙 ECO310 咖啡机进行改造。旨在为用户提供自制咖啡的独特体验，通过对 ECO310 的改造，使用户可以在家中轻松制作高质量的个性化咖啡。


目前有两个分支:
1. itcoffee one lite: 由于arduino nano只有有限的RAM和flash. so ,lite版本只包含简单的功能，用来控制eco310.
   video : 加热控制   看看【IT咖啡的作品】用arduino控制咖啡机的加热  https://v.douyin.com/iFGxaUU5/ S@Y.MJ 03/31 EHV:/
   
3. itcoffee one
   基于stm32F411, 将会包含全部的功能。 
    

这个项目灵感来着gaggiuino. 在此基础上
1. 去掉lcd部分. 因为gaggiuino的程序与lcd结合极深，不宜与移植到其他hmi设备
2. 增加modbus协议支持。只要支持modbus协议,传统的工控HMI，或者支持modbus的ipad，android程序都可以与之通讯
4. 软件模块化。 将把软件划分为加热控制模块和泵控模块以及io模块，这样可以容易扩展加热锅炉的数量。
5. 
## 使用说明

本项目 是 一个 刚开始的项目。

### 计划

1. 加热部分控制
2. 压力部分控制
3. pcb板子
4. Android IOS APP

## 贡献

我们欢迎并感谢任何形式的贡献。如果你想为项目做出贡献，请参考 [贡献指南]，提交问题、提议功能或进行代码贡献。

## 许可证

ITCoffee One 项目采用 MIT 许可证。

MIT License

版权所有（c）2024 <Michael Jiang>

特此免费授予任何获得本软件和相关文档文件（“软件”）副本的人，无论按何种方式和目的，许可软件使用、复制、修改、合并、出版、分发、再许可和/或销售的权利，但需遵守以下条件：

上述版权声明和本许可声明应包含在软件的所有副本或重要部分中。

本软件按“原样”提供，无任何形式的明示或暗示保证，包括但不限于对适销性、特定用途的适用性和非侵权性的保证。在任何情况下，作者或版权持有人均不对由本软件或本软件的使用或其他交易引起的任何索赔、损害或其他责任负责。


## 致谢

特别感谢 [Gaggiuino](https://github.com/Zer0-bit/gaggiuino) 项目，以及德龙 ECO310 咖啡机，我们的项目在设计和实现过程中受到了启发和参考。

## DIY 记录与讨论

我们鼓励大家在 [Issues](https://github.com/jiangxinghui/itcoffee-one/issues) 中记录 DIY 过程中的问题、想法和讨论。请在该部分分享你的经验，提出问题或与其他社区成员进行交流。让我们共同努力打造出更完美的 ITCoffee One！



