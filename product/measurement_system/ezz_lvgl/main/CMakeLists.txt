idf_component_register(
    SRCS
        "main.cpp"
        "mqtt.c"
        "CAN_comn.c"
        "math_rpm.c"

        #eez
        "ui/eez_agmo/src/ui/eez-flow_bak.cpp"
        "ui/eez_agmo/src/ui/screens.c"
        "ui/eez_agmo/src/ui/ui.c"
        "ui/eez_agmo/src/ui/images.c"
        "ui/eez_agmo/src/ui/styles.c"
        "ui/eez_agmo/src/ui/ui_font_maru_buri_regular_kr16.c"
        "ui/eez_agmo/src/ui/ui_font_maru_buri_regular_kr20.c"
        "ui/eez_agmo/src/ui/ui_font_maru_buri_regular_kr28.c"
        "ui/eez_agmo/src/ui/ui_image_agmo"
        "ui/eez_agmo/src/ui/ui_image_rda.c"
        "ui/eez_agmo/src/ui/ui_image_arc_main.c"
        "ui/eez_agmo/src/ui/ui_image_arc_indicator.c"
        "ui/eez_agmo/src/ui/ui_image_motor.c"
        # (optional)
        #bridge
        "ui/glue/ui_bridge.c"
        "ui/glue/ui_adaptor.c"
        # "480_800_16_bmp.c"
    INCLUDE_DIRS
        "."
        "ui/eez_agmo/src/ui"
        "ui/glue"
)
