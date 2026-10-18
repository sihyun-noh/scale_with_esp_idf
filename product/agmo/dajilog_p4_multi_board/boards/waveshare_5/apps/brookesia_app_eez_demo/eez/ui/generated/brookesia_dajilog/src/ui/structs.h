#ifndef EEZ_LVGL_UI_STRUCTS_H
#define EEZ_LVGL_UI_STRUCTS_H

#include "eez-flow.h"

#include <stdint.h>
#include <stdbool.h>

#include "vars.h"

using namespace eez;

enum FlowStructures {
    FLOW_STRUCTURE__3_AXIS_STRUCTS = 16384,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS = 16385
};

enum FlowArrayOfStructures {
    FLOW_ARRAY_OF_STRUCTURE__3_AXIS_STRUCTS = 81920,
    FLOW_ARRAY_OF_STRUCTURE__3_AXIS_VEHICLE_STRUCTS = 81921
};

enum _3_axis_structsFlowStructureFields {
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_0 = 0,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_1 = 1,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_2 = 2,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_3 = 3,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_4 = 4,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_5 = 5,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_6 = 6,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_7 = 7,
    FLOW_STRUCTURE__3_AXIS_STRUCTS_NUM_FIELDS
};

enum _3_axis_vehicle_structsFlowStructureFields {
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_0 = 0,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_1 = 1,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_2 = 2,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_3 = 3,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_4 = 4,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_5 = 5,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_6 = 6,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_7 = 7,
    FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_NUM_FIELDS
};

struct _3_axis_structsValue {
    Value value;
    
    _3_axis_structsValue() {
        value = Value::makeArrayRef(FLOW_STRUCTURE__3_AXIS_STRUCTS_NUM_FIELDS, FLOW_STRUCTURE__3_AXIS_STRUCTS, 0);
    }
    
    _3_axis_structsValue(Value value) : value(value) {}
    
    operator Value() const { return value; }
    
    operator bool() const { return value.isArray(); }
    
    const char *data_0() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_0].getString();
    }
    void data_0(const char *data_0) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_0] = StringValue(data_0);
    }
    
    const char *data_1() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_1].getString();
    }
    void data_1(const char *data_1) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_1] = StringValue(data_1);
    }
    
    const char *data_2() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_2].getString();
    }
    void data_2(const char *data_2) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_2] = StringValue(data_2);
    }
    
    const char *data_3() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_3].getString();
    }
    void data_3(const char *data_3) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_3] = StringValue(data_3);
    }
    
    const char *data_4() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_4].getString();
    }
    void data_4(const char *data_4) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_4] = StringValue(data_4);
    }
    
    const char *data_5() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_5].getString();
    }
    void data_5(const char *data_5) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_5] = StringValue(data_5);
    }
    
    const char *data_6() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_6].getString();
    }
    void data_6(const char *data_6) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_6] = StringValue(data_6);
    }
    
    const char *data_7() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_7].getString();
    }
    void data_7(const char *data_7) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_STRUCTS_FIELD_DATA_7] = StringValue(data_7);
    }
};

typedef ArrayOf<_3_axis_structsValue, FLOW_ARRAY_OF_STRUCTURE__3_AXIS_STRUCTS> ArrayOf_3_axis_structsValue;
struct _3_axis_vehicle_structsValue {
    Value value;
    
    _3_axis_vehicle_structsValue() {
        value = Value::makeArrayRef(FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_NUM_FIELDS, FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS, 0);
    }
    
    _3_axis_vehicle_structsValue(Value value) : value(value) {}
    
    operator Value() const { return value; }
    
    operator bool() const { return value.isArray(); }
    
    const char *data_0() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_0].getString();
    }
    void data_0(const char *data_0) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_0] = StringValue(data_0);
    }
    
    const char *data_1() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_1].getString();
    }
    void data_1(const char *data_1) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_1] = StringValue(data_1);
    }
    
    const char *data_2() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_2].getString();
    }
    void data_2(const char *data_2) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_2] = StringValue(data_2);
    }
    
    const char *data_3() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_3].getString();
    }
    void data_3(const char *data_3) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_3] = StringValue(data_3);
    }
    
    const char *data_4() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_4].getString();
    }
    void data_4(const char *data_4) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_4] = StringValue(data_4);
    }
    
    const char *data_5() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_5].getString();
    }
    void data_5(const char *data_5) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_5] = StringValue(data_5);
    }
    
    const char *data_6() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_6].getString();
    }
    void data_6(const char *data_6) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_6] = StringValue(data_6);
    }
    
    const char *data_7() {
        return value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_7].getString();
    }
    void data_7(const char *data_7) {
        value.getArray()->values[FLOW_STRUCTURE__3_AXIS_VEHICLE_STRUCTS_FIELD_DATA_7] = StringValue(data_7);
    }
};

typedef ArrayOf<_3_axis_vehicle_structsValue, FLOW_ARRAY_OF_STRUCTURE__3_AXIS_VEHICLE_STRUCTS> ArrayOf_3_axis_vehicle_structsValue;

#endif /*EEZ_LVGL_UI_STRUCTS_H*/