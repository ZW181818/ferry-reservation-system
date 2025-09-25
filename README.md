# SuperFerry Reservation System  
超级渡轮预订系统

---

##  Overview | 项目概述
This project is a **C++ console-based reservation system** designed for managing ferry sailings, vehicles, and reservations.  
It was developed as a **course project (CMPT 276: Introduction to Software Engineering)** to practice **modular design, file persistence, user interface flow, and system integration**.  

本项目是一个 **基于 C++ 控制台的渡轮预订系统**，用于管理航行、车辆和预订信息。  
它作为 **CMPT 276 软件工程课程项目** 开发，旨在实践 **模块化设计、数据持久化、用户交互流程与系统集成**。

---

##  Key Features | 核心功能
- **Reservation Management**  
  - Create new reservations, check in vehicles, and delete confirmed reservations.  
  - Handles vehicle dimensions, lane allocation, and prevents overbooking.  

  **预订管理**  
  - 创建新预订、车辆登船登记、删除已确认的预订。  
  - 支持车辆尺寸校验、车道空间分配，避免超额预订。  

- **Sailing Management**  
  - Create and delete sailings with associated ferry information.  
  - Real-time lane capacity tracking (High Ceiling / Low Ceiling).  
  - Automatic onboard vehicle count updates.  

  **航行管理**  
  - 创建和删除航行，并绑定相应的渡轮信息。  
  - 实时追踪车道容量（高顶 / 低顶）。  
  - 自动更新已登船车辆数量。  

- **Ferry Management**  
  - Create ferry vessels with user-defined lane capacities.  
  - Enforces name uniqueness and auto-converts lowercase names to uppercase.  
  - Allows deletion of ferries with confirmation.  

  **渡轮管理**  
  - 创建渡轮并指定车道容量。  
  - 自动将小写船名转为大写并防止重名。  
  - 可删除已有渡轮并提供确认提示。  

- **Sailing Report**  
  - Paginated report of all sailings with vehicle statistics.  
  - Built-in performance timer to measure report generation speed (tested with 15,000+ sailings).  

  **航行报告**  
  - 分页显示所有航行及车辆统计信息。  
  - 内置性能计时器，可测试大规模数据（超过 15,000 条记录）的加载速度。  

- **System Utilities**  
  - Reset entire system (clear all records).  
  - Data persistence via ASM (Abstract Storage Modules).  

  **系统工具**  
  - 一键重置系统（清空所有记录）。  
  - 通过 ASM（抽象存储模块）实现数据持久化。  

---

##  Project Structure | 项目结构
