#pragma once

constexpr double au_to_meters      = 5.2917721092e-11;
constexpr double barn_to_sqrmeters = 1.0e-28;

constexpr double speed_of_light    = 137.035999139;

constexpr double au_to_barns = au_to_meters * au_to_meters / barn_to_sqrmeters;

constexpr double au_to_ev = 27.211385;

//conversion from 1 W/m^2 to the unit of E^2 in atomic units; I = eps_0 c E^2 /2 
constexpr double intensity_to_au = 3.50944758e16;

