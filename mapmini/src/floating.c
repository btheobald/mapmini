


    if(t_lookup == tile->t_lookup) {
        printf("Redraw: \n", tile->number_of_ways);
        for(int w = 0; w < tile->number_of_ways; w++) {
            if(view->subtile & tile->way_data[w].subtile_bitmap)
                g_draw_way(&(tile->way_data[w]), 0, tile->way_data[w].tag_ids[0], view);
        }
        return 1;
    } else {
        printf("Reload\n");
        arena_free(arena);
        tile->t_lookup = t_lookup;
    }

        // Get tile latlong origin.    
    int lon = tilex2long(xyz->x, xyz->z);
    int lat = tiley2lat(xyz->y, xyz->z);
  
    int lon1 = tilex2long(xyz->x+1, xyz->z);
    int lat1 = tiley2lat(xyz->y+1, xyz->z);
  
    int londiff = abs(lon1 - lon);
    int latdiff = abs(lat1 - lat);
  
    printf("tile   origin: %d, %d\n", lat, lon);
    printf("tile + origin: %d, %d\n", lat1, lon1);
    printf("tile differences: %d, %d\n", latdiff, londiff);

    // Tile X Mercator Scaling Factor
    int x_pix = lon_to_x(londiff, 1);
    int y_pix = lat_to_y(latdiff, 1);
    float x_mercator = ((float)x_pix/y_pix);
    float fit_scale = (float)y_pix/view->scale;
    int x_fit = lon_to_x(londiff, x_mercator*fit_scale);
    int y_fit = lat_to_y(latdiff, fit_scale);
    
    printf("scale diff to: %d, %d\n", y_pix, x_pix);
    printf("scale factors: %d, %f, %f (%f)\n", view->scale, fit_scale, x_mercator*fit_scale, x_mercator);
    printf("fit diff tile: %d, %d\n", y_fit, x_fit);