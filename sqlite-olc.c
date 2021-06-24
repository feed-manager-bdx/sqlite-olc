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

#ifdef __WIN32
#include "/usr/include/sqlite3ext.h"
#else
#include <sqlite3ext.h>
#endif
#include <string.h>
#include <math.h>
#include <olc.h>

SQLITE_EXTENSION_INIT1

#define DEG_TO_RAD(x) ((x) / 180 * M_PI)
#define EARTH_RADIUS_AT_EQTR 6378137
#define EARTH_RADIUS_AT_POLE 6356752

static double get_earth_radius(double lat) 
{
    return sqrt(
	(
            pow(pow(EARTH_RADIUS_AT_EQTR, 2) * cos(lat), 2) +
	    pow(pow(EARTH_RADIUS_AT_POLE, 2) * sin(lat), 2)
	) / (
            pow(EARTH_RADIUS_AT_POLE * cos(lat), 2) +
	    pow(EARTH_RADIUS_AT_POLE * sin(lat), 2)
	)
    );
}

static int get_center(sqlite3_value *value, OLC_LatLon *center)
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
    
    return (int) (get_earth_radius(l->lat) * v_c);
}

#define E_ARG_NULL -1
#define E_ARG_BAD_TYPE -2

static int checkParams(int argc, sqlite3_value **argv, int expectedType)
{
	int hasNull = 0;
        for (int i = 0; i < argc; ++i) {
                int type = sqlite3_value_type(argv[i]);
                if (type == SQLITE_NULL) {
                        hasNull = 1;
                } else if (type != expectedType) {
			return E_ARG_BAD_TYPE;	
                }
        }
        if (hasNull) {
                return E_ARG_NULL;
        }
	return 0;
}

static int checkParamsOrError(sqlite3_context *context, int argc, sqlite3_value **argv, int expectedType)
{
	switch (checkParams(argc, argv, expectedType)) {
                case E_ARG_BAD_TYPE:
                        sqlite3_result_error(context, "Invalid parameter type", -1);
                        return 1;
                case E_ARG_NULL:
                        sqlite3_result_null(context);
                        return 1;
		default:
			return 0;
        }
}

static void olcDistanceFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	if (checkParamsOrError(context, argc, argv, SQLITE_TEXT) != 0) {
		return;
	}

	OLC_LatLon l_center, r_center;
	if (get_center(argv[0], &l_center) != 0 || get_center(argv[1], &r_center) != 0) {
		sqlite3_result_error(context, "Failed to parse OLC", -2);
		return;
	}

	int distance = get_distance(&l_center, &r_center);
	sqlite3_result_int(context, distance); // in meters

}

static void geoDistanceFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	if (checkParamsOrError(context, argc, argv, SQLITE_FLOAT) != 0) {
		return;
	}

	OLC_LatLon l = { 
		.lat = sqlite3_value_double(argv[0]), 
		.lon = sqlite3_value_double(argv[1]) 
	};
	OLC_LatLon r = { 
		.lat = sqlite3_value_double(argv[2]), 
		.lon = sqlite3_value_double(argv[3])
	};

	int distance = get_distance(&l, &r);
	sqlite3_result_int(context, distance); // in meters
}

static void olcGeoDistanceFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	if (checkParamsOrError(context, 1, argv+0, SQLITE_TEXT) != 0) {
		return;
	}
	if (checkParamsOrError(context, 2, argv+1, SQLITE_FLOAT) != 0) {
		return;
	}

	OLC_LatLon r = { 
		.lat = sqlite3_value_double(argv[1]), 
		.lon = sqlite3_value_double(argv[2])
       	};

	OLC_LatLon l_center;
	if (get_center(argv[0], &l_center) != 0) {
		sqlite3_result_error(context, "Failed to parse OLC", -2);
		return;
	}

	int distance = get_distance(&l_center, &r);
	sqlite3_result_int(context, distance); // in meters
}

int sqlite3_extension_init(
	sqlite3 *db,
	char **pzErrMsg,
	const sqlite3_api_routines *pApi
){
	SQLITE_EXTENSION_INIT2(pApi)
	sqlite3_create_function(db, "olc_distance", 2, SQLITE_ANY, 0, olcDistanceFunc, 0, 0);
	sqlite3_create_function(db, "geo_distance", 4, SQLITE_ANY, 0, geoDistanceFunc, 0, 0);
	sqlite3_create_function(db, "olc_geo_distance", 3, SQLITE_ANY, 0, olcGeoDistanceFunc, 0, 0);
	return 0;
}
