CREATE TABLE User(
    username varchar(255) PRIMARY KEY,
    status bool NOT NULL,
    private_guid varchar(64) NOT NULL
);

CREATE TABLE message ( 
    messageGUID VARCHAR(64) NOT NULL PRIMARY KEY,
    sender_username VARCHAR(255) NOT NULL,
    reciever_username VARCHAR(255) NOT NULL,
    content VARCHAR(150) NOT NULL,
    timestamp INT NOT NULL,
    isGroup bool NOT NULL
);

CREATE TABLE groups (
    groupId varchar(64) NOT NULL PRIMARY KEY,
    groupName VARCHAR(150) NOT NULL,
    createDate INT NOT NULL,
    isActive bool
);

CREATE TABLE user_group (
    username varchar(255) NOT NULL,
    groupId varchar(64) NOT NULL,
    FOREIGN KEY (username) REFERENCES user(username),
    FOREIGN KEY (groupId) REFERENCES groups(groupId),
    PRIMARY KEY (username, groupId)
);