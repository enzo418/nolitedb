-- SQLITE3

-- INSERT
PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE `collection` (`id` INTEGER PRIMARY KEY,`name` varchar(255));
INSERT INTO collection VALUES(1,'persona');
INSERT INTO collection VALUES(2,'_persona_contact');
CREATE TABLE `document` (`id` INTEGER PRIMARY KEY,`coll_id` int,FOREIGN KEY (coll_id) REFERENCES collection(id));
INSERT INTO document VALUES(1,1);
INSERT INTO document VALUES(2,2);
CREATE TABLE `property` (`id` INTEGER PRIMARY KEY,`coll_id` int,`name` varchar(255),`type` int,FOREIGN KEY (coll_id) REFERENCES collection(id));
INSERT INTO property VALUES(1,1,'contact',4);
INSERT INTO property VALUES(2,2,'address',1);
INSERT INTO property VALUES(3,2,'email',1);
INSERT INTO property VALUES(4,2,'phone',1);
INSERT INTO property VALUES(5,1,'name',1);
CREATE TABLE `value_int` (`id` INTEGER PRIMARY KEY,`doc_id` int,`prop_id` int,`value` int,FOREIGN KEY (doc_id) REFERENCES document(id),FOREIGN KEY (prop_id) REFERENCES property(id));
CREATE TABLE `value_double` (`id` INTEGER PRIMARY KEY,`doc_id` int,`prop_id` int,`value` DOUBLE,FOREIGN KEY (doc_id) REFERENCES document(id),FOREIGN KEY (prop_id) REFERENCES property(id));
CREATE TABLE `value_string` (`id` INTEGER PRIMARY KEY,`doc_id` int,`prop_id` int,`value` varchar(255),FOREIGN KEY (doc_id) REFERENCES document(id),FOREIGN KEY (prop_id) REFERENCES property(id));
INSERT INTO value_string VALUES(1,2,2,'fake st 99');
INSERT INTO value_string VALUES(2,2,3,'fake@fake.fake');
INSERT INTO value_string VALUES(3,2,4,'12344');
INSERT INTO value_string VALUES(4,1,5,'enzo a.');
CREATE TABLE `value_object` (`id` INTEGER PRIMARY KEY,`doc_id` int,`prop_id` int,`sub_coll_id` int,`sub_doc_id` int,FOREIGN KEY (doc_id) REFERENCES document(id),FOREIGN KEY (prop_id) REFERENCES property(id),FOREIGN KEY (sub_coll_id) REFERENCES collection(id)FOREIGN KEY (sub_doc_id) REFERENCES document(id));
INSERT INTO value_object VALUES(1,1,1,2,2);
COMMIT;

-- SELECT QUERY EXAMPLE
select 'name_5'.value as 'n_5', 'email_3'.value as 'em_3' from 'document' __doc
 left join value_string as 'name_5' on (__doc.id = 'name_5'.doc_id and 'name_5'.prop_id = 5)
 left join value_object as 'contact_1' on (__doc.id = 'contact_1'.doc_id and 'contact_1'.prop_id = 1)
 left join value_string as 'email_3' on ('contact_1'.sub_doc_id = 'email_3'.doc_id and 'email_3'.prop_id = 3);

-- as you can see, in properties from an object we join the value with value_object.sub_doc_id instead of the regular
-- document alias.id


-- Get ids from inner documents
select 
        __doc.id as 'id_1',  -- this is the document id from the root collection user
		'name_5'.value as 'name_5', 
		'contact_1'.sub_doc_id as 'id_2', -- this is the document id of subcollection contact
		'address_2'.value as 'address_2', 'email_3'.value as 'email_3', 'phone_4'.value as 'phone_4' 
		from 'document' __doc
 left join value_string as 'name_5' on (__doc.id = 'name_5'.doc_id and 'name_5'.prop_id = 5)
 left join value_object as 'contact_1' on (__doc.id = 'contact_1'.doc_id and 'contact_1'.prop_id = 1)
 left join value_string as 'address_2' on ('contact_1'.sub_doc_id = 'address_2'.doc_id and 'address_2'.prop_id = 2)
 left join value_string as 'email_3' on ('contact_1'.sub_doc_id = 'email_3'.doc_id and 'email_3'.prop_id = 3)
 left join value_string as 'phone_4' on ('contact_1'.sub_doc_id = 'phone_4'.doc_id and 'phone_4'.prop_id = 4)