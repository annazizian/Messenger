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
    timestamp INT NOT NULL
);

CREATE TABLE group (
  groupId INT NOT NULL PRIMARY KEY,
  groupName VARCHAR(150) NOT NULL,
  createDate DATE NOT NULL,
  isActive CHAR(1),
);

CREATE TABLE user_group (
    userID INT NOT NULL FOREIGN KEY REFERENCES user(ID),
    groupId INT NOT NULL FOREIGN KEY REFERENCES group(groupID),
    CONSTRAINT user_groupID PRIMARY KEY (userID, groupID)
);

CREATE TABLE user_to_group (
	Id INT NOT NULL PRIMARY KEY IDENTITY(0,1),
    senderID INT NOT NULL FOREIGN KEY REFERENCES user(ID),
    recieverID INT NOT NULL FOREIGN KEY REFERENCES group(groupID),
	messageID INT NOT NULL FOREIGN KEY REFERENCES message(messageID)
);