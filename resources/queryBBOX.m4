[out:json][bbox:BOTTOM_LEFT_LAT, BOTTOM_LEFT_LON, TOP_RIGHT_LAT, TOP_RIGHT_LON];
(
  way['highway'='motorway'];
  way['highway'='motorway_link'];
  way['highway'='trunk'];
  way['highway'='trunk_link'];
  way['highway'='primary'];
  way['highway'='primary_link'];
  way['highway'='secondary'];
  way['highway'='secondary_link'];
  way['highway'='tertiary'];
  way['highway'='tertiary_link'];
  way['highway'='residential'];
  way['highway'='living_street'];
  way['highway'='unclassified'];
  way['highway'='service'];
  way['highway'='track']; 
  way['vehicle'='destination']; 
  way['cycleway:right'];
)->.car;
(.car; >;)->.car;
.car out;
