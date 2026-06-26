var express = require('express')
var router = express.Router()
var mongoose = require('mongoose')
var models = reqlib('database').models
var moment = require('moment')
const { ObjectId } = require('mongoose').Types

const fs = require('fs');
const path = require('path');
const { v4: uuidv4 } = require('uuid');
const currentDirectory = __dirname;
const parentDirectory = path.resolve(currentDirectory, '..', '..');
const savePathImageAvatar = `${parentDirectory}/images/avatar`;

module.exports = () => {
    router.post('/update', async (req, res) => {
        try {
            const UserID = req.UserID
            const { FullName } = req.body
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }

            let avatar = null
            if (!fs.existsSync(savePathImageAvatar)) {
                fs.mkdirSync(savePathImageAvatar, { recursive: true });
            }

            for (const file of req.files) {
                if (file.fieldname === 'avatar') {
                    const extension = file.originalname.split('.').pop();
                    const nameFile = uuidv4();
                    const fullPath = path.join(savePathImageAvatar, `${nameFile}.${extension}`);
                    fs.writeFileSync(fullPath, file.buffer);
                    avatar = `/avatar/${nameFile}.${extension}`;
                }
            }
            const updateObject = {};

            if (FullName) {
                updateObject.FullName = FullName;
            }

            if (avatar) {
                updateObject.Avatar = avatar;
            }
            updateObject.UpdateAt = moment().toDate();
            if (Object.keys(updateObject).length > 0) {
                await models.Users.updateOne({ _id: user._id }, updateObject);
            }
            return res.status(200).json({ status: 1, data: null, message: "update success" })

        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.get('/info', async (req, res) => {
        try {
            const UserID = req.UserID
            let user = await models.Users.findOne({ _id: new ObjectId(UserID) }).exec()
            if (user == null) {
                return res.status(400).json({ status: 0, data: null, message: 'User not found' })
            }
            return res.status(200).json({
                status: 1, data: {
                    Username: user?.Username,
                    FullName: user?.FullName,
                    Avatar: user?.Avatar
                }, message: ''
            })
        } catch (error) {
            return res.status(400).json({ status: 0, data: null, message: error.message })
        }
    })

    router.post('/nickname/update', async (req, res) => {
        try {
            const UserID = req.UserID;
            const { FriendID, Nickname } = req.body;

            if (!FriendID || !Nickname) {
                return res.status(400).json({ status: 0, message: "Thiếu FriendID hoặc Nickname" });
            }

            // 1. Kiểm tra UserID (người thực hiện) có tồn tại không
            const ownerExists = await models.Users.findOne({ _id: new ObjectId(UserID) });
            if (!ownerExists) {
                return res.status(400).json({ status: 0, message: "User not found" });
            }

            // 2. (Tùy chọn) Kiểm tra FriendID có tồn tại trong hệ thống không
            const friendExists = await models.Users.findOne({ _id: new ObjectId(FriendID) });
            if (!friendExists) {
                return res.status(400).json({ status: 0, message: "Friend not found" });
            }

            // 3. Xây dựng updateObject và thực hiện update
            const updateObject = {
                Nickname: Nickname,
                UpdateAt: new Date()
            };

            await models.Nickname.findOneAndUpdate(
                { UserID: new ObjectId(UserID), FriendID: new ObjectId(FriendID) },
                updateObject,
                { upsert: true, new: true }
            );

            return res.status(200).json({ status: 1, message: "Cập nhật thành công" });
        } catch (error) {
            return res.status(400).json({ status: 0, message: error.message });
        }
    })

    router.get('/nickname/list', async (req, res) => {
        try {
            const UserID = req.UserID;

            // 1. Kiểm tra xem UserID có tồn tại trong hệ thống không
            const userExists = await models.Users.findOne({ _id: new ObjectId(UserID) });
            if (!userExists) {
                return res.status(400).json({ status: 0, message: "User not found" });
            }

            // 2. Lấy danh sách nickname của User đó
            const list = await models.Nickname.find({ UserID: new ObjectId(UserID) });
            
            return res.status(200).json({ status: 1, data: list });
        } catch (error) {
            return res.status(400).json({ status: 0, message: error.message });
        }
    })

    router.post('/nickname/delete', async (req, res) => {
        try {
            const UserID = req.UserID;
            const { FriendID } = req.body; 

            const userExists = await models.Users.findOne({ _id: new ObjectId(UserID) });
            if (!userExists) {
                return res.status(400).json({ status: 0, message: "User not found" });
            }

            if (!FriendID) {
                return res.status(400).json({ status: 0, message: "Thiếu FriendID" });
            }

            const result = await models.Nickname.deleteOne({ 
                UserID: new ObjectId(UserID), 
                FriendID: new ObjectId(FriendID) 
            });

            if (result.deletedCount === 0) {
                return res.status(404).json({ status: 0, message: "Không tìm thấy biệt danh để xóa" });
            }

            return res.status(200).json({ status: 1, message: "Đã xóa biệt danh thành công" });
        } catch (error) {
            return res.status(400).json({ status: 0, message: error.message });
        }
    })

    return router
}