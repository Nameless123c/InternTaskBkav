const mongoose = require('mongoose');
const { ObjectId } = mongoose.Types;
const MessageModel = require('./database/schemas/message.js');

// Kết nối tới database
mongoose.connect('mongodb://localhost:27017/CHAT');

const seedMessages = async () => {
    try {
        // ID đã có từ trước của Thiên và Quang
        const ThienID = new ObjectId('6a1c3e39aa3a6a2db1f9c181');
        const QuangID = new ObjectId('6a1c3e43aa3a6a2db1f9c184');

        const messages = [
            // Thiên gửi cho Quang
            {
                UserID: ThienID,
                FriendID: QuangID,
                Content: 'Xin chào! Chúng ta làm quen nhé.',
                isSend: 1, // 1 là đã gửi
                CreatedAt: new Date(Date.now() - 60000), // 1 phút trước
                Files: [],
                Images: []
            },
            // Quang hồi đáp cho Thiên
            {
                UserID: QuangID,
                FriendID: ThienID,
                Content: 'Ok Thiên nhé, hôm nào cafe đi!',
                isSend: 1, // Đánh dấu đã gửi (theo logic mới)
                CreatedAt: new Date(), // Ngay bây giờ
                Files: [],
                Images: []
            }
        ];

        // Xóa dữ liệu cũ và chèn dữ liệu mới
        await MessageModel.deleteMany({});
        await MessageModel.insertMany(messages);

        console.log('Đã tạo hội thoại giữa Thiên và Quang thành công!');
        process.exit();
    } catch (err) {
        console.error('Lỗi khi seed dữ liệu:', err);
        process.exit(1);
    }
};

seedMessages();