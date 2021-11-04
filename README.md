These files are a legacy and non abstracted implementation with FATFS functions baked in, this was tested running on an STM32H723 and demonstrated promising loading and rendering performance.

This was tested using a file generated using osmosis from raw OSM.PBF downloaded from geofabrik, configured to include only ways marked as highways, which reduces file-size by more than a factor of 10. This also makes the job of parsing the file substantially less complex. This version does not differentiate bettween filled polygons and lines, and does not correctly handle multipolygons.
