/**
 * SQLite Open Location Code extension
 *
 * Copyright (C) 2021 Feed Manager (Labelium Group) 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <sqlite3ext.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <olc.h>

SQLITE_EXTENSION_INIT1

#define DEG_TO_RAD(x) ((x) / 180 * M_PI)

static int getCenter(sqlite3_value *value, OLC_LatLon *center)
{
        OLC_CodeArea area;
	const char *str = (const char *) sqlite3_value_text(value);
        if (! OLC_Decode(str, strlen(str), &area)) {
                return -1;
        }

        OLC_GetCenter(&area, center);
	return 0;
}

int get_distance(OLC_LatLon *l, OLC_LatLon *r) 
{
    double dLat = DEG_TO_RAD(l->lat - r->lat),
	   dLon = DEG_TO_RAD(l->lon - r->lon),
	   cos_l_lat = cos(DEG_TO_RAD(l->lat)),
	   cos_r_lat = cos(DEG_TO_RAD(r->lat));

    double v_a = pow(sin(dLat/2), 2) + cos_l_lat * cos_r_lat * pow(sin(dLon/2), 2);
    double v_c = 2 * atan2(sqrt(v_a), sqrt(1-v_a));
    
    return (int) (6371e3 * v_c);
}

static void olcDistanceFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[1]) == SQLITE_NULL){
		sqlite3_result_null(context);
		return;
	}

	OLC_LatLon l_center, r_center;
	if (getCenter(argv[0], &l_center) != 0 || getCenter(argv[1], &r_center) != 0) {
		sqlite3_result_null(context);
                return;
	}

	int distance = get_distance(&l_center, &r_center);
	sqlite3_result_int(context, distance); // in meters
}

int sqlite3_extension_init(
	sqlite3 *db,
	char **pzErrMsg,
	const sqlite3_api_routines *pApi
){
	SQLITE_EXTENSION_INIT2(pApi)
	sqlite3_create_function(db, "olc_distance", 2, SQLITE_ANY, 0, olcDistanceFunc, 0, 0);
	return 0;
}
