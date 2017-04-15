
CREATE GENERATOR vkgroupInc;

SET TERM ^ ;
CREATE TRIGGER vkgroupInc FOR vkgroup ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (vkgroupInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR vkpostInc;

SET TERM ^ ;
CREATE TRIGGER vkpostInc FOR vkpost ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (vkpostInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR vktrackInc;

SET TERM ^ ;
CREATE TRIGGER vktrackInc FOR vktrack ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (vktrackInc, 1);
END^
SET TERM ; ^



CREATE GENERATOR vkphotoInc;

SET TERM ^ ;
CREATE TRIGGER vkphotoInc FOR vkphoto ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (vkphotoInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR commentsInc;

SET TERM ^ ;
CREATE TRIGGER commentsInc FOR comments ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (commentsInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR vkdocInc;

SET TERM ^ ;
CREATE TRIGGER vkdocInc FOR vkdoc ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (vkdocInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR eventtypeInc;

SET TERM ^ ;
CREATE TRIGGER eventtypeInc FOR eventtype ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (eventtypeInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR eventInc;

SET TERM ^ ;
CREATE TRIGGER eventInc FOR event ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (eventInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR circleInc;

SET TERM ^ ;
CREATE TRIGGER circleInc FOR circle ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (circleInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR albumInc;

SET TERM ^ ;
CREATE TRIGGER albumInc FOR album ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (albumInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR fandomInc;

SET TERM ^ ;
CREATE TRIGGER fandomInc FOR fandom ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (fandomInc, 1);
END^
SET TERM ; ^


CREATE GENERATOR trackInc;

SET TERM ^ ;
CREATE TRIGGER trackInc FOR track ACTIVE
BEFORE INSERT POSITION 1
AS
BEGIN
    if (new.ID is null) then
    new.ID = gen_id (trackInc, 1);
END^
SET TERM ; ^
