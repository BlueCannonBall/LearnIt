/* This file was generated by ODB, object-relational mapping (ORM)
 * compiler for C++.
 */

DROP TABLE IF EXISTS `Term`;

DROP TABLE IF EXISTS `Deck_terms`;

DROP TABLE IF EXISTS `Deck`;

DROP TABLE IF EXISTS `User`;

CREATE TABLE `User` (
  `username` VARCHAR(255) NOT NULL PRIMARY KEY,
  `email` TEXT NOT NULL,
  `password_hash` TEXT NOT NULL,
  `creation_date` BIGINT NOT NULL)
 ENGINE=InnoDB;

CREATE TABLE `Deck` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `owner` VARCHAR(255) NOT NULL,
  `name` TEXT NOT NULL,
  `creation_date` BIGINT NOT NULL
  /*
  CONSTRAINT `Deck_owner_fk`
    FOREIGN KEY (`owner`)
    REFERENCES `User` (`username`)
  */)
 ENGINE=InnoDB;

CREATE TABLE `Deck_terms` (
  `object_id` BIGINT UNSIGNED NOT NULL,
  `index` BIGINT UNSIGNED NOT NULL,
  `value` BIGINT UNSIGNED NOT NULL,
  CONSTRAINT `Deck_terms_object_id_fk`
    FOREIGN KEY (`object_id`)
    REFERENCES `Deck` (`id`)
    ON DELETE CASCADE)
 ENGINE=InnoDB;

CREATE INDEX `object_id_i`
  ON `Deck_terms` (`object_id`);

CREATE INDEX `index_i`
  ON `Deck_terms` (`index`);

CREATE TABLE `Term` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `term` TEXT NOT NULL,
  `definition` TEXT NOT NULL)
 ENGINE=InnoDB;

/*
ALTER TABLE `Deck_terms`
  ADD CONSTRAINT `Deck_terms_value_fk`
    FOREIGN KEY (`value`)
    REFERENCES `Term` (`id`)
*/

