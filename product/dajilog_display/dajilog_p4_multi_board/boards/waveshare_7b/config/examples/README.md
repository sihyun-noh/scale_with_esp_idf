# Synthetic calibration CSV example

`SAMPLE_SYNTHETIC_CALIBRATION_DO_NOT_USE.csv` follows the draft controller CSV columns in the MAIN Dashboard document, section 7.3. All discharge values are invented, not measured. `valid=1` now means each synthetic point is usable for *test calculations only*, as requested; it does **not** mean the table is measured or approved. `product_code=DEMO_ONLY` and `product_lot=SYNTHETIC` mark the file as a fixture.

With `CONFIG_DAJILOG_CALIBRATION_SAMPLE_TEST=y`, firmware registers four unapproved catalog *summaries* mirroring this CSV. The CSV itself is not parsed or embedded, and no measurement points are loaded into the calculator. The UI can display sample candidates, but selection/confirmation, NVS mapping, automatic CAN output and production RUN remain blocked. Before real operation, replace the fixture with measured and reviewed data and complete the approval workflow.
