#
# MySQL tables for the roll-dice demo
#

CREATE SCHEMA demo;

CREATE TABLE demo.roll(
  rollid INTEGER NOT NULL AUTO_INCREMENT,
  dice INTEGER,
  PRIMARY KEY(rollid));

CREATE USER 'app'@'%' IDENTIFIED BY 'demo123';

GRANT INSERT on demo.roll TO 'app'@'%';

