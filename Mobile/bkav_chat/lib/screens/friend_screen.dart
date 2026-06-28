import 'package:flutter/material.dart';
import '../services/session_manager.dart';
import '../services/api_service.dart';
import 'dart:convert';

class FriendScreen extends StatefulWidget {
  const FriendScreen({super.key});

  @override
  State<FriendScreen> createState() => _FriendScreenState();
}

class _FriendScreenState extends State<FriendScreen> {
  final TextEditingController _nicknameController = TextEditingController();
  String _errorMessage = "";

  @override
  void initState() {
    super.initState();
    final friendId = SessionManager.selectedFriend!.friendId;
    _nicknameController.text = SessionManager.nicknameMap[friendId] ?? "";
  }

  Future<void> _updateNickname() async {
    String newNickname = _nicknameController.text.trim();
    String friendId = SessionManager.selectedFriend!.friendId;

    if (newNickname.isEmpty){
      setState(() {
        _errorMessage = "Hãy nhập biệt danh cho đối phương";
      });
      return;
    }

    String url = "http://10.0.2.2:8888/api/user/nickname/update";

    Map<String, dynamic> data = {
      "FriendID": friendId,
      "Nickname": newNickname,
    };

    String res = await ApiService.sendPostRequest(url, data, token: SessionManager.token);

    if (res.isEmpty){
      setState(() {
        _errorMessage = "Lỗi kết nối mạng!";
      });
      return;
    }

    var jsonRes = jsonDecode(res);

    String msg = jsonRes["message"];

    setState(() {
      _errorMessage = msg;
      SessionManager.nicknameMap[friendId] = newNickname;
    });

    return;
  }

  @override
  Widget build(BuildContext context) {
    final friend = SessionManager.selectedFriend;

    final String displayName = SessionManager.nicknameMap.containsKey(friend!.friendId)
        ? SessionManager.nicknameMap[friend.friendId]!
        : friend.fullName;

    return Scaffold(
      backgroundColor: Colors.white,
      appBar: AppBar(
        toolbarHeight: 70.0,
        backgroundColor: Colors.white,
        elevation: 1,
        leading: IconButton(
          icon: const Icon(Icons.arrow_back, color: Colors.black),
          onPressed: () => Navigator.pop(context),
        ),
        title: const Text("Tùy chọn", style: TextStyle(color: Colors.black, fontSize: 18)),
      ),
      // Dùng Padding để đẩy xuống thay vì Center -> linh hoạt hơn
      body: SingleChildScrollView(
        padding: const EdgeInsets.only(top: 120.0),
        child: SizedBox(
          width: double.infinity, // Giúp Column căn giữa theo chiều ngang màn hình
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              CircleAvatar(
                radius: 50,
                backgroundImage: AssetImage(friend!.avatar),
              ),
              const SizedBox(height: 20),
              Text(
                displayName,
                style: const TextStyle(
                  fontSize: 24,
                  fontWeight: FontWeight.bold,
                  color: Colors.black87,
                ),
              ),

              const SizedBox(height: 30),

              Padding(
                padding: const EdgeInsets.symmetric(horizontal: 40.0), // Tạo khoảng cách lề hai bên
                child: TextField(
                  controller: _nicknameController,
                  decoration: InputDecoration(
                    labelText: 'Nhập biệt danh...', // Nhãn hiển thị
                    border: OutlineInputBorder(
                      borderRadius: BorderRadius.circular(10.0), // Bo góc cho đẹp
                    ),
                  ),
                ),
              ),

              const SizedBox(height: 30),

              Row(
                mainAxisAlignment: MainAxisAlignment.center, // Căn giữa hàng nút theo chiều ngang
                children: [
                  ElevatedButton(
                    onPressed: () {
                      _updateNickname();
                    },
                    style: ElevatedButton.styleFrom(
                      backgroundColor: const Color.fromRGBO(28, 127, 217, 1.0), // RGB(28, 127, 217)
                      foregroundColor: const Color.fromRGBO(255, 255, 255, 1.0), // Màu chữ trắng
                      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 12), // Tăng kích thước nút
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(8), // Bo góc nhẹ
                      ),
                    ),
                    child: const Text("Thay đổi biệt danh"),
                  ),
                  const SizedBox(width: 20),
                  TextButton(
                    onPressed: () {
                    },
                    style: TextButton.styleFrom(
                      foregroundColor: const Color.fromRGBO(4, 125, 231, 1.0), // RGB(4, 125, 231)
                    ),
                    child: const Text(
                      "Xóa biệt danh",
                    ),
                  ),
                ],
              ),

              const SizedBox(height: 20),

              if (_errorMessage.isNotEmpty)
                Center(
                  child: Padding(
                    padding: const EdgeInsets.all(8.0),
                    child: Text(
                      _errorMessage,
                      style: const TextStyle(color: Colors.red, fontSize: 16),
                    ),
                  ),
                ),


            ],
          ),
        ),
      ),
    );
  }
}