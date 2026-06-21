import 'dart:async'; // Bắt buộc phải import thư viện này
import 'package:flutter/material.dart';
import '../services/session_manager.dart';
import 'dart:convert';
import '../services/api_service.dart';
import '../models/message.dart';

class ChatfriendScreen extends StatefulWidget {
  const ChatfriendScreen({super.key});

  @override
  State<ChatfriendScreen> createState() => _ChatFriendScreen();
}

class _ChatFriendScreen extends State<ChatfriendScreen> {
  bool _isLoading = true;
  bool _isFirstLoad = true;
  List<Message> _messageList = [];
  final TextEditingController _textController = TextEditingController();
  final ScrollController _scrollController = ScrollController();
  Timer? _messageTimer; // Khai báo Timer

  @override
  void initState() {
    super.initState();
    _loadInitialData(forceScroll: true);

    // Thiết lập timer cập nhật mỗi 1 giây
    _messageTimer = Timer.periodic(const Duration(seconds: 1), (timer) {
      _loadInitialData(); // Gọi update ngầm
    });
  }

  Future<void> _loadInitialData({bool forceScroll = false}) async {
    bool isAtBottom = false;
    if (_scrollController.hasClients) {
      isAtBottom = _scrollController.position.maxScrollExtent - _scrollController.offset < 50;
    }

    List<Message> data = await _getMessage();

    if (mounted) {
      // Chỉ cập nhật UI nếu có sự thay đổi tin nhắn để tránh giật màn hình
      if (data.length != _messageList.length) {
        setState(() {
          _messageList = data;
          _isLoading = false;
        });

        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (forceScroll || _isFirstLoad || isAtBottom) {
            _scrollToBottom();
            _isFirstLoad = false;
          }
        });
      }
    }
  }

  Future<List<Message>> _getMessage() async {
    String url = "http://10.0.2.2:8888/api/message/get-message?FriendID=${SessionManager.selectedFriend!.friendId}";
    String token = SessionManager.token!;

    String res = await ApiService.sendGetRequest(url, token: token);
    if (res.isEmpty) return [];

    var jsonRes = jsonDecode(res);
    if (jsonRes["status"] == 1) {
      final List<dynamic> dataList = jsonRes["data"];
      return dataList.map((item) {
        return Message(
          id: item["id"],
          content: item["Content"] ?? "",
          images: [],
          files: [],
          isSend: item["isSend"] ?? 0,
          createdAt: item["CreatedAt"] ?? "",
          messageType: item["MessageType"] ?? 0,
        );
      }).toList();
    }
    return [];
  }

  Future<void> _sendMessage() async {
    String content = _textController.text.trim();
    if (content.isEmpty) return;

    String url = "http://10.0.2.2:8888/api/message/send-message";
    Map<String, dynamic> data = {
      "FriendID": SessionManager.selectedFriend!.friendId,
      "Content": content,
    };

    String res = await ApiService.sendPostRequest(url, data, token: SessionManager.token!);

    if (res.isNotEmpty) {
      var jsonRes = jsonDecode(res);
      if (jsonRes["status"] == 1) {
        _textController.clear();
        await _loadInitialData(forceScroll: true);
      }
    }
  }

  void _scrollToBottom() {
    if (_scrollController.hasClients) {
      _scrollController.animateTo(
        _scrollController.position.maxScrollExtent,
        duration: const Duration(milliseconds: 300),
        curve: Curves.easeOut,
      );
    }
  }

  @override
  void dispose() {
    _messageTimer?.cancel(); // Dừng timer quan trọng!
    _textController.dispose();
    _scrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoading) {
      return const Scaffold(body: Center(child: CircularProgressIndicator()));
    }

    return Scaffold(
      appBar: AppBar(
        toolbarHeight: 80.0,
        backgroundColor: Colors.white,
        elevation: 2,
        automaticallyImplyLeading: false,
        titleSpacing: 0,
        title: Row(
          children: [
            IconButton(
              icon: const Icon(Icons.arrow_back, color: Colors.black),
              onPressed: () => Navigator.pop(context),
            ),
            CircleAvatar(radius: 20, backgroundImage: AssetImage(SessionManager.selectedFriend!.avatar)),
            const SizedBox(width: 12),
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(SessionManager.selectedFriend!.fullName, style: const TextStyle(fontSize: 16, color: Colors.black)),
                Text(SessionManager.selectedFriend!.isOnline ? "Trực tuyến" : "Ngoại tuyến", style: const TextStyle(fontSize: 12, color: Colors.grey)),
              ],
            ),
          ],
        ),
      ),
      body: Column(
        children: [
          Expanded(
            child: ListView.builder(
              controller: _scrollController,
              padding: const EdgeInsets.only(top: 10, left: 10, right: 10, bottom: 20),
              itemCount: _messageList.length,
              itemBuilder: (context, index) {
                final message = _messageList[index];
                bool isMe = message.messageType == 1;

                return Align(
                  alignment: isMe ? Alignment.centerRight : Alignment.centerLeft,
                  child: Container(
                    margin: const EdgeInsets.symmetric(vertical: 4, horizontal: 8),
                    padding: const EdgeInsets.all(12),
                    constraints: BoxConstraints(
                      maxWidth: MediaQuery.of(context).size.width * 0.5,
                    ),
                    decoration: BoxDecoration(
                      color: isMe ? Colors.teal : Colors.grey[300],
                      borderRadius: BorderRadius.circular(12),
                    ),
                    child: Text(
                      message.content,
                      style: TextStyle(color: isMe ? Colors.white : Colors.black),
                    ),
                  ),
                );
              },
            ),
          ),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
            decoration: const BoxDecoration(
              color: Colors.white,
              border: Border(top: BorderSide(color: Colors.black12)),
            ),
            child: Row(
              children: [
                Image.asset('icon/LOL.png', width: 30),
                const SizedBox(width: 8),
                Expanded(
                  child: Container(
                    height: 40,
                    decoration: BoxDecoration(
                      color: Colors.grey[200],
                      borderRadius: BorderRadius.circular(20),
                    ),
                    child: TextField(
                      controller: _textController,
                      decoration: const InputDecoration(
                        hintText: "Nhập tin nhắn",
                        hintStyle: TextStyle(
                          fontStyle: FontStyle.italic,
                          color: Colors.grey,
                        ),
                        border: InputBorder.none,
                        contentPadding: EdgeInsets.symmetric(horizontal: 15, vertical: 10),
                      ),
                    ),
                  ),
                ),
                const SizedBox(width: 8),
                IconButton(
                  icon: Image.asset('icon/Polygon.png', width: 30),
                  onPressed: _sendMessage,
                ),
                const SizedBox(width: 4),
                Image.asset('icon/Attach.png', width: 30),
                const SizedBox(width: 4),
                Image.asset('icon/Image.png', width: 30),
              ],
            ),
          ),
        ],
      ),
    );
  }
}