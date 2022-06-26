#ifndef ELEVATOR_PROTOCOL_PARSE_H
#define ELEVATOR_PROTOCOL_PARSE_H

#include "mainwindow.h"

/* CMD DEFINE */

/* QUERY */
#define QUERY (124)                /* 查询电梯 */
#define QUERY_ELEVATOR_STATUS (32) /* 获取电梯状态 */
#define STRESS_TEST (34)           /* 电梯压力测试 */
#define ERROR_CODE (35)            /* 错误码上报 */
#define SLAVE_OTA (37)             /* 从机OTA中 */
#define SLAVE_OTA_DONE (38)        /* OTA完成校验 */
#define HEART_PACKET (43)          /* 从机心跳包 */
#define CONNECTION_STATUS (45)     /* 连接状态 */
#define GET_TAG_INFO (46)          /* 获取从机标签文件 */
#define STRESS_TEST_START (47)     /* 压力测试启动 */

/* CONFIG */
#define ELEVATOR_CONFIG (125)
#define CONFIG_INIT (32)       /* 电梯进入配置模式 */
#define SET_FLOOR_NUM (33)     /* 设置电梯楼层数 */
#define SET_DOOR_INFO (34)     /* 设置电梯门信息信息 */
#define SET_FLOOR_EPC (35)     /* 设置楼层标签值 */
#define SET_FLOOR_IO (36)      /* 设置单个楼层IO(未使用) */
#define SET_FLOORS_IO (37)     /* 设置所有楼层IO */
#define CONFIG_OK (38)         /* 确认电梯配置 */
#define UPDATE (39)            /* 从机OTA升级 */
#define IP_CONFIG (40)         /* IP配置(未使用) */
#define ELE_RESET (41)         /* 电梯复位 */
#define FLOOR_TAG_QUALITY (44) /* 标签质量检测 */
#define FLOOR_TAG_CLEAR (45)   /* 清除楼层标签 */

/* TODO */
#define ELEVATOR_CMD (126)
#define EXTERNAL_CALL (32)
#define CONTROL_DOOR (34)
#define CANCEL_TASK (35)
#define INTERNAL_CALL (40)
#define PREEMPT_ELEVATOR (41)
#define RELEASE_ELEVATOR (48)

/* flag ops */
#define FLAG_ENABLE (1)
#define FLAG_DISABLE (0)

#define NO_POS (-1)
/* cmd result */
#define CMD_SUCCESS (1)
#define CMD_FAIL (0)

/* current elevator protocol is 3 */
//#define elevatorProtocolLen 1 //current elevator protocol is 3

#define dataLen 10
#define timeLen 12
#define cmdLen 3
#define toDoLen 2

/* select language */
//#define English

/* LORA data direction */
enum Source { Master = 0, Slave = 1, Robot = 2 };

/* data enum Source */
enum dataType { LORA = 100, MQTT_SEND = 101, MQTT_RECV = 102, OTHER = 0 };

/*define struct */
/* LORA data struct */
struct loraData {
    short direction;
    short cmd;
    short toDo;
    short retryTimes;  // Lora retry times
};
/* mqtt data struct */
struct mqttData {
    QString action;
    int floor;
    int status;
    int result;
    int callType;  // callType: 1:external call 2:internal
    int ele_id;
    QString sendTaskNo;
    QString recTaskNo;
};

/* log data type*/
enum logDataType {
    LORA_INFO,
    LORA_WARN,
    LORA_ERROR,
};

/* call type enum */
enum callType { NO_CALL = 0, EXTERNAL = 1, INTERNAL = 2 };

/* door control */
enum door { CLOSE_DOOR = 0, OPEN_DOOR = 1 };

/* elevator data struct */
struct ELEVATOR {
    /* common data */
    QString date;
    QString time;
    uint processId;
    uint messageId;
    int eleId;
    int floor;
    int openDoorTime;
    short robotId;
    bool taskCut;
    bool isArrived;
    bool isFilter;
    bool is_current_robot;
    int filterEleId;
    int filterRobotId;
    int logType;
    int elvatorProtocolVersion;
    /* LORA data */
    loraData lora;
    /* mqtt data */
    mqttData mqtt;
};

int strDateToBuf(QString &rawData, QByteArray *frame);
QString elevator_lora_data_parse(QByteArray &rawData);

/* LORA data parse function */
QString loraRawDataParse(ELEVATOR &ele, QByteArray &rawData, QString &result);
/* LORA cmd data parse function */
QString loraCmdParse(ELEVATOR &ele, QByteArray &rawData, QString &result);

/* elevator protocol:7C/7E/7D */
QString queryCmd(ELEVATOR &ele, QByteArray &rawData, QString &result);   /* query:7C 124 */
QString configCmd(ELEVATOR &ele, QByteArray &rawData, QString &result);  /* query:7D 125 */
QString controlCmd(ELEVATOR &ele, QByteArray &rawData, QString &result); /* control:7E 126 */

/* query elevator cmd*/
QString queryElevatorStatus(ELEVATOR &ele, QByteArray &rawData, QString &result); /* query elevator status */
QString slaveHeartPacket(ELEVATOR &ele, QByteArray &rawData, QString &result);    /* slave heart packet */

/* query elevator cmd*/
QString initElevatorConfig(ELEVATOR &ele, QByteArray &rawData, QString &result);
QString confirmElevatorConfig(ELEVATOR &ele, QByteArray &rawData, QString &result);

/*elevator control cmd*/
QString preemptElevator(ELEVATOR &ele, QByteArray &rawData, QString &result); /* preempt the elevator */
QString externalCall(ELEVATOR &ele, QByteArray &rawData, QString &result);    /* external call elevator */
QString controlDoor(ELEVATOR &ele, QByteArray &rawData, QString &result);     /* control door */
QString internalCall(ELEVATOR &ele, QByteArray &rawData, QString &result);    /* internal call elevator */
QString releaseElevator(ELEVATOR &ele, QByteArray &rawData, QString &result); /* release the elevator */
QString cancelTheTask(ELEVATOR &ele, QByteArray &rawData, QString &result);   /* cancel the elevator task */

#endif /* ELEVATOR_PROTOCOL_PARSE_H */
