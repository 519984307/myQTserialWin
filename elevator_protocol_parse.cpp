#include "elevator_protocol_parse.h"

struct ELEVATOR ele;

QString elevatorCmdParse(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    switch (ele.lora.cmd) {
        case QUERY:
            result = queryCmd(ele, rawData, result);
            break;
        case ELEVATOR_CONFIG:
            result = configCmd(ele, rawData, result);
            break;
        case ELEVATOR_CMD:
            result = controlCmd(ele, rawData, result);
            break;
        default:
            break;
    }

    return result;
}

QString elevator_lora_data_parse(QByteArray &rawData) {
    QString res                = "test";
    ele.elvatorProtocolVersion = rawData.at(0);
    ele.processId = (uint8_t)rawData.at(4) << 24 | (uint8_t)rawData.at(3) << 16 | (uint8_t)rawData.at(2) << 8 |
                    (uint8_t)rawData.at(1);
    ele.messageId = (uint8_t)rawData.at(8) << 24 | (uint8_t)rawData.at(7) << 16 | (uint8_t)rawData.at(6) << 8 |
                    (uint8_t)rawData.at(5);
    ele.eleId          = (uint8_t)rawData.at(10) << 8 | (uint8_t)rawData.at(9);
    ele.lora.direction = (uint8_t)rawData.at(11) >> 4;
    ele.robotId        = (uint8_t)rawData.at(11) & 0x0F;
    ele.lora.cmd       = (uint8_t)rawData.at(12);
    ele.lora.toDo      = (uint8_t)rawData.at(13);
    res                = elevatorCmdParse(ele, rawData, res);
    qDebug() << ele.processId;
    qDebug() << ele.messageId;
    qDebug() << ele.eleId;
    qDebug() << ele.lora.direction;
    qDebug() << ele.robotId;
    qDebug() << ele.lora.cmd;
    qDebug() << ele.lora.toDo;
    return res;
}

QString configCmd(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    switch (ele.lora.toDo) {
        /* init elevator config */
        case CONFIG_INIT:
            result = initElevatorConfig(ele, rawData, result);
            break;
        /* confirm elevator config */
        case CONFIG_OK:
            result = confirmElevatorConfig(ele, rawData, result);
            break;
        default:
            break;
    }
    return result;
}

QString controlCmd(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    switch (ele.lora.toDo) {
        /* external call the elevator */
        case EXTERNAL_CALL:
            result = externalCall(ele, rawData, result);
            break;
            /* control door */
        case CONTROL_DOOR:
            result = controlDoor(ele, rawData, result);
            break;
            /* cancel elevator */
        case CANCEL_TASK:
            result = cancelTheTask(ele, rawData, result);
            break;
            /* internal call the elevator */
        case INTERNAL_CALL:
            result = internalCall(ele, rawData, result);
            break;
            /* preempt the elevator */
        case PREEMPT_ELEVATOR:
            result = preemptElevator(ele, rawData, result);
            break;
            /* release the elevator */
        case RELEASE_ELEVATOR:
            result = releaseElevator(ele, rawData, result);
            break;
        default:
            break;
    }
    return result;
}

/* elevator config function set */
QString initElevatorConfig(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        if (res_val) {
            ans = (res_val == CMD_SUCCESS) ? "成功." : "失败,已在配置模式中.";
        }
        result = "接收(从机):进入配置模式" + ans;
    } else if (ele.lora.direction == Master) {
        result = "接收(主机):进入配置模式.";
    }
    return result;
}

QString confirmElevatorConfig(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        result  = "接收(从机):确认电梯配置" + ans;
    } else if (ele.lora.direction == Master) {
        result = "接收(主机):确认电梯配置.";
    }
    return result;
}

/* elevator control function set */

/* preempt the elevator */
QString preemptElevator(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        if (!ele.robotId) {
            result = "接收(从机):后台抢占电梯" + ans;
        } else {
            result = "接收(从机):" + QString::number((ele.robotId)) + "号机器人抢占" + ans;
        }
        if (!res_val) {
            result += "已抢占机器人ID:" + QString::number((uint8_t)rawData.at(18)) + ".";
        }
    } else if (ele.lora.direction == Master) {
        if (!ele.robotId) {
            result = "接收(主机):后台抢占电梯.";
        } else {
            result = "接收(主机):" + QString::number((ele.robotId)) + "号机器人抢占电梯.";
        }
    } else {
        result = "接收(机器):" + QString::number((ele.robotId)) + "号机器人抢占电梯.";
    }
    return result;
}

/* external call the elevator */
QString externalCall(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    static QString ans, floor, openDoorTime;
    if (ele.lora.direction == Slave) {
        auto callRes = (uint8_t)rawData.at(17);
        ans          = (callRes == CMD_SUCCESS) ? "成功." : "失败.";
        result       = "接收(从机):外呼" + ans;
        if (callRes == CMD_SUCCESS) ele.isArrived = 0;
    } else {
        if (ele.lora.direction == Robot)
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
QString internalCall(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    static QString ans, floor, openDoorTime;
    if (ele.lora.direction == Slave) {
        auto callRes = (uint8_t)rawData.at(17);
        ans          = (callRes == CMD_SUCCESS) ? "成功." : "失败.";
        result       = "接收(从机):内呼" + ans;
        if (callRes == CMD_SUCCESS) ele.isArrived = 0;
    } else {
        if (ele.lora.direction == Robot)
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
QString controlDoor(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    int res_val    = 0;
    QString method = "";
    res_val        = (uint8_t)rawData.at(17);
    method         = (res_val == CMD_SUCCESS) ? "开门." : "关门.";
    if (ele.lora.direction == Slave) {
        result  = "接收(从机):" + method;
        res_val = (uint8_t)rawData.at(18);
        method  = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        result += method;
    } else if (ele.lora.direction == Master) {
        result = "接收(主机)请求电梯:" + method + QString::number((uint8_t)rawData.at(18)) + "秒.";
    } else {
        result = "接收(机器)请求电梯:" + method + QString::number((uint8_t)rawData.at(18)) + "秒.";
    }
    return result;
}

/* release the elevator */
QString releaseElevator(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        if (!ele.robotId) {
            result = "接收(从机):后台释放电梯" + ans;
        } else {
            result = "接收(从机):" + QString::number((ele.robotId)) + "号机器人释放" + ans;
        }
    } else {
        if (ele.lora.direction == Robot)
            result = "接收(机器):" + QString::number((ele.robotId)) + "号机器人释放电梯.";
        else {
            if (!ele.robotId) {
                result = "接收(主机):后台释放电梯.";
            } else {
                result = "接收(主机):" + QString::number((ele.robotId)) + "号机器人释放电梯.";
            }
        }
    }
    return result;
}

/* elevator query function set */
QString queryCmd(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    switch (ele.lora.toDo) {
        /* get elevator status */
        case QUERY_ELEVATOR_STATUS:
            result = queryElevatorStatus(ele, rawData, result);
            break;
        /* slave heart packet */
        case HEART_PACKET:
            result = slaveHeartPacket(ele, rawData, result);
            break;
        default:
            break;
    }
    return result;
}

/* cancel elevator task */
QString cancelTheTask(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    int res_val = 0;
    QString ans = "";
    if (ele.lora.direction == Slave) {
        res_val = (uint8_t)rawData.at(17);
        ans     = (res_val == CMD_SUCCESS) ? "成功." : "失败.";
        result  = "接收(从机):" + QString::number((ele.robotId)) + "号机器人取消任务" + ans;
    } else {
        if (ele.lora.direction == Robot)
            result = "接收(机器):" + QString::number((ele.robotId)) + "号机器人取消任务.";
        else {
            result = "接收(主机):" + QString::number((ele.robotId)) + "号机器人取消任务.";
        }
    }
    return result;
}

/* query thr elevator status 7C 20 */
QString queryElevatorStatus(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    static QString floor, run_status, arrived_status;
    if (ele.lora.direction == Slave) {
        floor          = "楼层:" + QString::number((uint8_t)rawData.at(17));
        run_status     = ",运行状态:" + QString::number((uint8_t)rawData.at(18));
        arrived_status = ",到达状态:" + QString::number((uint8_t)rawData.at(19));
        result         = "接收(从机):电梯状态:" + floor + run_status + arrived_status + ".";
    } else if (ele.lora.direction == Robot) {
        result = "接收(机器):查询电梯状态.";
    } else {
        result = "接收(主机):查询电梯状态.";
    }
    return result;
}

/* slave heart packet 7C 2B */
QString slaveHeartPacket(ELEVATOR &ele, QByteArray &rawData, QString &result) {
    if (ele.lora.direction == Slave) {
        result = "接收(从机):收到从机心跳包.";
    }
    return result;
}
