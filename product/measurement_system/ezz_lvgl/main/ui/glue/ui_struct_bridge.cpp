#include "ui_struct_bridge.h"

#include "ui/eez_agmo/src/ui/eez-flow.h"
#include "ui/eez_agmo/src/ui/structs.h"
#include "ui/eez_agmo/src/ui/vars.h"

using namespace eez;
using namespace eez::flow;

void ui_struct_set_field(int index, const char *text) {
  const char *value = text ? text : "";

  Value current = getGlobalVariable(FLOW_GLOBAL_VARIABLE__3_AXIS_STRUCT);
  _3_axis_structsValue axis = current.isArray() ? _3_axis_structsValue(current) : _3_axis_structsValue();

  switch (index) {
    case 0: axis.data_0(value); break;
    case 1: axis.data_1(value); break;
    case 2: axis.data_2(value); break;
    case 3: axis.data_3(value); break;
    case 4: axis.data_4(value); break;
    case 5: axis.data_5(value); break;
    case 6: axis.data_6(value); break;
    case 7: axis.data_7(value); break;
    default: return;
  }

  setGlobalVariable(FLOW_GLOBAL_VARIABLE__3_AXIS_STRUCT, axis);
}

void ui_struct_vehicle_field(int index, const char *text) {
  const char *value = text ? text : "";

  Value current = getGlobalVariable(FLOW_GLOBAL_VARIABLE__3_AXIS_VEHICLE_STRUCT);
  _3_axis_structsValue axis = current.isArray() ? _3_axis_structsValue(current) : _3_axis_structsValue();

  switch (index) {
    case 0: axis.data_0(value); break;
    case 1: axis.data_1(value); break;
    case 2: axis.data_2(value); break;
    case 3: axis.data_3(value); break;
    case 4: axis.data_4(value); break;
    case 5: axis.data_5(value); break;
    case 6: axis.data_6(value); break;
    case 7: axis.data_7(value); break;
    default: return;
  }

  setGlobalVariable(FLOW_GLOBAL_VARIABLE__3_AXIS_VEHICLE_STRUCT, axis);
}
