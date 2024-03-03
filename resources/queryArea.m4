[out:json];
way(around: CIRCLE_METERS,BOTTOM_LEFT_LAT, BOTTOM_LEFT_LON) -> .car;
(
 way.car['highway'='motorway'];
 way.car['highway'='motorway_link'];
 way.car['highway'='trunk'];
 way.car['highway'='trunk_link'];
 way.car['highway'='primary'];
 way.car['highway'='primary_link'];
 way.car['highway'='secondary'];
 way.car['highway'='secondary_link'];
 way.car['highway'='tertiary'];
 way.car['highway'='tertiary_link'];
 way.car['highway'='residential'];
 way.car['highway'='living_street'];
 way.car['highway'='unclassified'];
 way.car['highway'='service'];
 way.car['highway'='track']; 
 way.car['vehicle'='destination']; 
 way.car['cycleway:right'];
)->.car;
(.car; >;)->.car;
.car out body;

