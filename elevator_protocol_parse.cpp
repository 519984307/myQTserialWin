#include "elevator_protocol_parse.h"

struct ELEVATOR ele_rx;

QString elevatorCmdParse(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    switch (ele_rx.lora.cmd) {
        case QUERY:
            result = queryCmd(ele_rx, rawData, result);
            break;
        case ELEVATOR_CONFIG:
            result = configCmd(ele_rx, rawData, result);
            break;
        case ELEVATOR_CMD:
            result = controlCmd(ele_rx, rawData, result);
            break;
        default:
            break;
    }

    return result;
}

QString elevator_lora_data_parse(QByteArray &rawData) {
    QString res                   = "";
    ele_rx.elvatorProtocolVersion = rawData.at(0);
    ele_rx.processId = (uint8_t)rawData.at(4) << 24 | (uint8_t)rawData.at(3) << 16 | (uint8_t)rawData.at(2) << 8 |
                       (uint8_t)rawData.at(1);
    ele_rx.messageId = (uint8_t)rawData.at(8) << 24 | (uint8_t)rawData.at(7) << 16 | (uint8_t)rawData.at(6) << 8 |
                       (uint8_t)rawData.at(5);
    ele_rx.eleId          = (uint8_t)rawData.at(10) << 8 | (uint8_t)rawData.at(9);
    ele_rx.lora.direction = (uint8_t)rawData.at(11) >> 4;
    ele_rx.robotId        = (uint8_t)rawData.at(11) & 0x0F;
    ele_rx.lora.cmd       = (uint8_t)rawData.at(12);
    ele_rx.lora.toDo      = (uint8_t)rawData.at(13);
    res                   = elevatorCmdParse(ele_rx, rawData, res);
    //    qDebug() << ele_rx.processId;
    //    qDebug() << ele_rx.messageId;
    //    qDebug() << ele_rx.eleId;
    //    qDebug() << ele_rx.lora.direction;
    //    qDebug() << ele_rx.robotId;
    //    qDebug() << ele_rx.lora.cmd;
    //    qDebug() << ele_rx.lora.toDo;
    /* 添加电梯ID字段 */
    if (!res.isEmpty()) {
        res += QString::asprintf("(%d)", ele_rx.eleId);
    }
    return res;
}

QString configCmd(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    switch (ele_rx.lora.toDo) {
        /* init elevator config */
        case CONFIG_INIT:
            result = initElevatorConfig(ele_rx, rawData, result);
            break;
        /* confirm elevator config */
        case CONFIG_OK:
            result = confirmElevatorConfig(ele_rx, rawData, result);
            break;
        default:
            break;
    }
    return result;
}

QString controlCmd(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    switch (ele_rx.lora.toDo) {
        /* external call the elevator */
        case EXTERNAL_CALL:
            result = externalCall(ele_rx, rawData, result);
            break;
            /* control door */
        case CONTROL_DOOR:
            result = controlDoor(ele_rx, rawData, result);
            break;
            /* cancel elevator */
        case CANCEL_TASK:
            result = cancelTheTask(ele_rx, rawData, result);
            break;
            /* reboot the elevator */
        case ELEVATOR_REBOOT:
            result = rebootTheElevator(ele_rx, rawData, result);
            /* internal call the elevator */
        case INTERNAL_CALL:
            result = internalCall(ele_rx, rawData, result);
            break;
            /* preempt the elevator */
        case PREEMPT_ELEVATOR:
            result = preemptElevator(ele_rx, rawData, result);
            break;
            /* release the elevator */
        case RELEASE_ELEVATOR:
            result = releaseElevator(ele_rx, rawData, result);
            break;
        default:
            break;
    }
    return result;
}

/* elevator config function set */
QString initElevatorConfig(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele_rx.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        if (res_val) {
            ans = (res_val == CMD_SUCCESS) ? "成功." : "失败,已在配置模式中.";
        }
        result = "接收(从机):进入配置模式" + ans;
    } else if (ele_rx.lora.direction == Master) {
        result = "接收(主机):进入配置模式.";
    }
    return result;
}

QString confirmElevatorConfig(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele_rx.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        result  = "接收(从机):确认电梯配置" + ans;
    } else if (ele_rx.lora.direction == Master) {
        result = "接收(主机):确认电梯配置.";
    }
    return result;
}

/* elevator control function set */

/* preempt the elevator */
QString preemptElevator(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele_rx.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        if (!ele_rx.robotId) {
            result = "接收(从机):后台抢占电梯" + ans;
        } else {
            result = "接收(从机):" + QString::number((ele_rx.robotId)) + "号机器人抢占" + ans;
        }
        if (!res_val) {
            result += "已抢占机器人ID:" + QString::number((uint8_t)rawData.at(18)) + ".";
        }
    } else if (ele_rx.lora.direction == Master) {
        if (!ele_rx.robotId) {
            result = "接收(主机):后台抢占电梯.";
        } else {
            result = "接收(主机):" + QString::number((ele_rx.robotId)) + "号机器人抢占电梯.";
        }
    } else {
        result = "接收(机器):" + QString::number((ele_rx.robotId)) + "号机器人抢占电梯.";
    }
    return result;
}

/* external call the elevator */
QString externalCall(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    static QString ans, floor, openDoorTime;
    if (ele_rx.lora.direction == Slave) {
        auto callRes = (uint8_t)rawData.at(17);
        ans          = (callRes == CMD_SUCCESS) ? "成功." : "失败.";
        result       = "接收(从机):外呼" + ans;
        if (callRes == CMD_SUCCESS) ele_rx.isArrived = 0;
    } else {
        if (ele_rx.lora.direction == Robot)
            ans = "接收(机器):外呼";
        else
            ans = "接收(主机):外呼";
        floor        = "楼层:" + QString::number((uint8_t)rawData.at(17));
        openDoorTime = ",开门时间:" + QString::number((uint8_t)rawData.at(18)) + "秒";
        result       = ans + floor + openDoorTime + ".";
    }
    return result;
}

/* internal call the elevator */
QString internalCall(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    static QString ans, floor, openDoorTime;
    if (ele_rx.lora.direction == Slave) {
        auto callRes = (uint8_t)rawData.at(17);
        ans          = (callRes == CMD_SUCCESS) ? "成功." : "失败.";
        result       = "接收(从机):内呼" + ans;
        if (callRes == CMD_SUCCESS) ele_rx.isArrived = 0;
    } else {
        if (ele_rx.lora.direction == Robot)
            ans = "接收(机器):内呼";
        else
            ans = "接收(主机):内呼";
        floor        = "楼层:" + QString::number((uint8_t)rawData.at(17));
        openDoorTime = ",开门时间:" + QString::number((uint8_t)rawData.at(18)) + "秒";
        result       = ans + floor + openDoorTime + ".";
    }
    return result;
}

/* control door */
QString controlDoor(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val    = 0;
    QString method = "";
    res_val        = (uint8_t)rawData.at(17);
    method         = (res_val == CMD_SUCCESS) ? "开门" : "关门";
    if (ele_rx.lora.direction == Slave) {
        result  = "接收(从机):" + method;
        res_val = (uint8_t)rawData.at(18);
        method  = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        result += method;
    } else if (ele_rx.lora.direction == Master) {
        result = "接收(主机)请求电梯:" + method + QString::number((uint8_t)rawData.at(18)) + "秒.";
    } else {
        result = "接收(机器)请求电梯:" + method + QString::number((uint8_t)rawData.at(18)) + "秒.";
    }
    return result;
}

/* release the elevator */
QString releaseElevator(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele_rx.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        if (!ele_rx.robotId) {
            result = "接收(从机):后台释放电梯" + ans;
        } else {
            result = "接收(从机):" + QString::number((ele_rx.robotId)) + "号机器人释放" + ans;
        }
    } else {
        if (ele_rx.lora.direction == Robot)
            result = "接收(机器):" + QString::number((ele_rx.robotId)) + "号机器人释放电梯.";
        else {
            if (!ele_rx.robotId) {
                result = "接收(主机):后台释放电梯.";
            } else {
                result = "接收(主机):" + QString::number((ele_rx.robotId)) + "号机器人释放电梯.";
            }
        }
    }
    return result;
}

/* elevator query function set */
QString queryCmd(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    switch (ele_rx.lora.toDo) {
        /* get elevator status */
        case QUERY_ELEVATOR_STATUS:
            result = queryElevatorStatus(ele_rx, rawData, result);
            break;
        /* stress test */
        case STRESS_TEST:
            result = stressTest(ele_rx, rawData, result);
            break;
        /* slave heart packet */
        case HEART_PACKET:
            result = slaveHeartPacket(ele_rx, rawData, result);
            break;
            /* stress test start */
        case STRESS_TEST_START:
            result = stressStart(ele_rx, rawData, result);
        default:
            break;
    }
    return result;
}

/* cancel elevator task */
QString cancelTheTask(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele_rx.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        result  = "接收(从机):" + QString::number((ele_rx.robotId)) + "号机器人取消任务" + ans;
    } else {
        if (ele_rx.lora.direction == Robot)
            result = "接收(机器):" + QString::number((ele_rx.robotId)) + "号机器人取消任务.";
        else {
            result = "接收(主机):" + QString::number((ele_rx.robotId)) + "号机器人取消任务.";
        }
    }
    return result;
}

/* reboot the elevator */
QString rebootTheElevator(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    if (ele_rx.lora.direction == Slave) {
        result = "接收(从机):请求主机重启";
    } else {
        if (ele_rx.lora.direction == Robot)
            result = "接收(机器):请求从机重启";
        else {
            result = "接收(主机):请求主从机重启.";
        }
    }
    return result;
}

/* query thr elevator status 7C 20 */
QString queryElevatorStatus(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    static QString floor, run_status, arrived_status;
    if (ele_rx.lora.direction == Slave) {
        floor          = "楼层:" + QString::number((uint8_t)rawData.at(17));
        run_status     = ",运行状态:" + QString::number((uint8_t)rawData.at(18));
        arrived_status = ",到达状态:" + QString::number((uint8_t)rawData.at(19));
        result         = "接收(从机):电梯状态:" + floor + run_status + arrived_status + ".";
    } else if (ele_rx.lora.direction == Robot) {
        result = "接收(机器):查询电梯状态.";
    } else {
        result = "接收(主机):查询电梯状态.";
    }
    return result;
}

/* slave stress test */
QString stressTest(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    if (ele_rx.lora.direction == Slave) {
        result = "接收(从机):lora压测中>流程ID:" + QString::number(ele_rx.processId) +
                 " 消息ID:" + QString::number(ele_rx.messageId) + " 楼层:" + QString::number((uint8_t)rawData.at(17)) +
                 " 运动状态:" + QString::number((uint8_t)rawData.at(18)) + ".";
    } else if (ele_rx.lora.direction == Master) {
        result = "接收(主机):lora压测中>流程ID:" + QString::number(ele_rx.processId) +
                 " 消息ID:" + QString::number(ele_rx.messageId) + ".";
    } else {
        result = "接收(机器):压测.";
    }
    return result;
}

/* slave heart packet 7C 2B */
QString slaveHeartPacket(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    if (ele_rx.lora.direction == Slave) {
        result = "接收(从机):收到从机心跳包.";
    }
    return result;
}

/* stress test start 7C 2F */
QString stressStart(ELEVATOR &ele_rx, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele_rx.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "失败,从机非空闲." : "成功.";
        result  = "接收(从机):压测启动" + ans;
    } else {
        if (ele_rx.lora.direction == Robot)
            result = "接收(机器):压测启动确认.";
        else {
            result = "接收(主机):压测启动确认.";
        }
    }
    return result;
}
