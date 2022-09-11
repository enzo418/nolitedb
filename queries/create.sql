CREATE TABLE `collection` (
  `id` INTEGER PRIMARY KEY,
  `name` varchar(255)
);

CREATE TABLE `property` (
  `id` INTEGER PRIMARY KEY,
  `coll_id` int,
  `name` varchar(255),
  `type` int,
  FOREIGN KEY (coll_id) REFERENCES collection(id)
);

CREATE TABLE `document` (
  `id` INTEGER PRIMARY KEY,
  `coll_id` int,
  FOREIGN KEY (coll_id) REFERENCES collection(id)
);

CREATE TABLE `value_string` (
  `id` INTEGER PRIMARY KEY,
  `doc_id` int,
  `prop_id` int,
  `value` varchar(255),
  FOREIGN KEY (doc_id) REFERENCES document(id),
  FOREIGN KEY (prop_id) REFERENCES property(id)
);

CREATE TABLE `value_int` (
  `id` INTEGER PRIMARY KEY,
  `doc_id` int,
  `prop_id` int,
  `value` int,
  FOREIGN KEY (doc_id) REFERENCES document(id),
  FOREIGN KEY (prop_id) REFERENCES property(id)
);
