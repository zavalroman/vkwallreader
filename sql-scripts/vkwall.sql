/*

*
*/

CREATE TABLE vkgroup
(
	id INTEGER,
	vk_id VARCHAR(50),
	vkdomain VARCHAR(50),
	name VARCHAR(100),
	
	PRIMARY KEY ( id )	
);

CREATE TABLE vkpost
(
	id INTEGER, --uniq post id in the table
	vk_id VARCHAR(50), --post id in a vk group
	vkgroup_id INTEGER,
	unix_time DECIMAL,
	post_text VARCHAR(4000),
	likes INTEGER,
	reposts INTEGER,
	comments INTEGER,
	
	photo_count SMALLINT,
	audio_count SMALLINT,
	
	FOREIGN KEY ( vkgroup_id ) REFERENCES vkgroup ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE vktrack
(
	id INTEGER,
	vk_id VARCHAR(50),
	--from_id VARCHAR(50),
	to_id VARCHAR(50), --owner_id
	artist VARCHAR(200),
	title VARCHAR(300),
	--duration INTEGER,
	--unix_time DECIMAL, --iru ka
	--url VARCHAR(100),
	--lyrics_id VARCHAR(50),
	--vkalbum_id INTEGER,
	--vkgenre_id INTEGER,
	vkpost_id INTEGER,

	FOREIGN KEY ( vkpost_id ) REFERENCES  vkpost ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE vkphoto
(
	id INTEGER,
	vk_id VARCHAR(20),
	photo_75 VARCHAR(500),
	photo_130 VARCHAR(500),
	photo_604 VARCHAR(500),
	photo_807 VARCHAR(500),
	photo_1280 VARCHAR(500),
	photo_2560 VARCHAR(500),
	width INTEGER,
	height INTEGER,
	text VARCHAR(1000),
	--unix_time DECIMAL,
	access_key VARCHAR(50),
	vkpost_id INTEGER,
	
	FOREIGN KEY ( vkpost_id ) REFERENCES vkpost ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE comments
(
	id INTEGER,
	vkpost_id INTEGER,
	commentator VARCHAR(20),
	text VARCHAR(1000),
	likes INTEGER,
	
	FOREIGN KEY ( vkpost_id ) REFERENCES vkpost ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE vkdoc
(
	id INTEGER,
	vk_id VARCHAR(20),
	owner_id VARCHAR(20),
	title VARCHAR(200),
	size INTEGER,
	ext VARCHAR(10),
	--url VARCHAR(200),
	access_key VARCHAR(50),
	vkpost_id INTEGER,
	
	FOREIGN KEY ( vkpost_id ) REFERENCES vkpost ( id ),
	PRIMARY KEY ( id )	
);

CREATE TABLE eventtype
(
	id INTEGER,
	title VARCHAR(100),
	--link_id INTEGER,
	
	--FOREIGN KEY ( link_id ) REFERENCES link ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE event
(
	id INTEGER,
	name VARCHAR(100),
	type_id INTEGER,
	occured CHAR(6), --format yymmdd
	--link_id INTEGER,
	
	FOREIGN KEY ( type_id ) REFERENCES eventtype ( id ),
	--FOREIGN KEY ( link_id ) REFERENCES link ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE circle
(
	id INTEGER,
	name_unic VARCHAR(100),
	name_roma VARCHAR(100),
	--link_id INTEGER,

	--FOREIGN KEY ( link_id ) REFERENCES link ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE album
(
	id INTEGER,
	title_unic VARCHAR(100),
	title_roma VARCHAR(100),
	circle_id INTEGER,
	track_count SMALLINT,
	duration INTEGER,
	catalog_n VARCHAR(100),
	event_id INTEGER,
	--link_id INTEGER,
	vktime DECIMAL,
	
	FOREIGN KEY ( circle_id ) REFERENCES circle ( id ),
	FOREIGN KEY ( event_id )  REFERENCES event  ( id ),
	--FOREIGN KEY ( link_id )   REFERENCES link   ( id ),
	PRIMARY KEY ( id )
);

CREATE TABLE fandom
(
	id INTEGER,
	name VARCHAR(100),
	--link_id INTEGER,
	
	PRIMARY KEY ( id )
);

CREATE TABLE track
(
	id INTEGER,
	vktrack_id INTEGER,
	--title_id INTEGER,
	title_unic VARCHAR(100),
	title_roma VARCHAR(100),
	album_id INTEGER,
	circle_id INTEGER,
	fandom_id INTEGER,
	duration INTEGER,
	bpm SMALLINT,
	
	FOREIGN KEY ( vktrack_id ) REFERENCES vktrack ( id ),
	--FOREIGN KEY ( title_id ) REFERENCES translit ( id ),
	FOREIGN KEY ( album_id )  REFERENCES album  ( id ),
	FOREIGN KEY ( circle_id ) REFERENCES circle ( id ),
	FOREIGN KEY ( fandom_id ) REFERENCES fandom ( id ),
	
	PRIMARY KEY ( id )
);
