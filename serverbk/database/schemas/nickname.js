const mongoose = require('mongoose')
const { ObjectId } = require('mongoose').Types

let Nickname = new mongoose.Schema({
    UserID: { type: ObjectId, required: true },
    
    FriendID: { type: ObjectId, required: true },
    
    Nickname: { type: String, required: true },
    
    CreatedAt: { type: Date, default: Date.now },
    UpdateAt: { type: Date, default: Date.now }
})

Nickname.index({ UserID: 1, FriendID: 1 }, { unique: true })

let NicknameModel = mongoose.model('nickname', Nickname)
module.exports = NicknameModel