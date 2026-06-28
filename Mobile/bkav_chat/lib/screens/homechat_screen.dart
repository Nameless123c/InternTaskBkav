import 'package:flutter/material.dart';
import 'dart:convert';
import '../services/api_service.dart';
import '../services/session_manager.dart';
import '../models/friend.dart';
import '../models/user.dart';
import 'chatfriend_screen.dart';
import 'dart:async';

class HomechatScreen extends StatefulWidget {
  const HomechatScreen({super.key});

  @override
  State<HomechatScreen> createState() => _HomechatScreen();
}

class _HomechatScreen extends State<HomechatScreen> {
  bool _isLoading = true;
  final TextEditingController _searchController = TextEditingController();
  List<Friend> _filteredList = [];
  Timer? _timer;

  @override
  void initState() {
    super.initState();
    _loadInitialData();
    _timer = Timer.periodic(const Duration(seconds: 10), (timer) {
      _refreshData();
    });
  }

  Future<void> _refreshData() async {
    // Chỉ cần gọi các hàm cập nhật dữ liệu (không cần clear danh sách để tránh nháy màn hình)
    await Future.wait([
      _getFriendList(),
      _getNickname(),
    ]);

    // Gọi setState để giao diện tự cập nhật với dữ liệu mới nhất
    if (mounted) {
      setState(() {
        _filteredList = SessionManager.friendList;
      });
    }
  }

  Future<void> _loadInitialData() async {
    await Future.wait([
      _getUserData(),
      _getFriendList(),
      _getNickname(),
    ]);
    if (mounted) {
      setState(() {
        _filteredList = SessionManager.friendList;
      });
      _isLoading = false;
      _searchController.addListener(_onSearchChanged);

    }
  }

  void _onSearchChanged() {
    String query = _searchController.text.toLowerCase();
    setState(() {
      _filteredList = SessionManager.friendList
          .where((friend) => friend.fullName.toLowerCase().contains(query))
          .toList();
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    _searchController.removeListener(_onSearchChanged);
    _searchController.dispose();
    super.dispose();
  }

  Future<void> _getUserData() async {
    String url = "http://10.0.2.2:8888/api/user/info";
    String token = SessionManager.token!;
    String res = await ApiService.sendGetRequest(url, token: token);
    if (res == "") return;
    var jsonRes = jsonDecode(res);
    if (jsonRes["status"] == 1) {
      var data = jsonRes["data"];

      // Xử lý làm sạch đường dẫn Avatar giống như bên friend list
      String rawAvatar = data.containsKey("Avatar") ? data["Avatar"] : 'avatar/default.jpg';
      String cleanAvatar = rawAvatar.startsWith('/') ? rawAvatar.substring(1) : rawAvatar;

      SessionManager.currentUser = User(
        username: data["Username"],
        fullName: data["FullName"],
        avatar: cleanAvatar, // Sử dụng biến đã làm sạch
      );
    }
  }

  Future<void> _getFriendList() async {
    SessionManager.friendList.clear();
    String url = "http://10.0.2.2:8888/api/message/list-friend";
    String token = SessionManager.token!;
    String res = await ApiService.sendGetRequest(url, token: token);
    if (res == "") return;
    var jsonRes = jsonDecode(res);
    if (jsonRes["status"] == 1) {
      List<dynamic> dataList = jsonRes["data"];
      for (var item in dataList) {
        // Xử lý chuỗi avatar: Lấy giá trị, nếu bắt đầu bằng '/' thì xóa nó đi
        String rawAvatar = item.containsKey("Avatar") ? item["Avatar"] : 'avatar/default.jpg';
        String cleanAvatar = rawAvatar.startsWith('/') ? rawAvatar.substring(1) : rawAvatar;

        SessionManager.friendList.add(Friend(
          friendId: item["FriendID"],
          fullName: item["FullName"],
          isOnline: item["isOnline"],
          avatar: cleanAvatar, // Lưu giá trị đã làm sạch vào model
        ));
      }
    }
  }

  Future<void> _getNickname() async {
    SessionManager.friendList.clear();
    String url = "http://10.0.2.2:8888/api/user/nickname/list";
    String token = SessionManager.token!;
    String res = await ApiService.sendGetRequest(url, token: token);

    if (res == "") return;

    var jsonRes = jsonDecode(res);
    if (jsonRes["status"] == 1) {
      SessionManager.nicknameMap.clear();

      if (jsonRes.containsKey("data") && jsonRes["data"] is List) {
        List<dynamic> dataList = jsonRes["data"];

        for (var item in dataList) {
          String friendId = item["FriendID"]?.toString() ?? "";
          String nickname = item["Nickname"]?.toString() ?? "";

          // 3. Lưu vào Map nếu friendId hợp lệ
          if (friendId.isNotEmpty) {
            SessionManager.nicknameMap[friendId] = nickname;
          }
        }

      }

    }
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoading) {
      return const Scaffold(body: Center(child: CircularProgressIndicator()));
    }

    return Scaffold(
      body: Padding(
        padding: const EdgeInsets.all(20.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const SizedBox(height: 50),

            // --- HEADER CỐ ĐỊNH ---
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text("Bkav Chat", style: TextStyle(fontSize: 28, fontWeight: FontWeight.bold, color: Color(0xFF6783E7))),
                CircleAvatar(
                  radius: 30,
                  backgroundImage: SessionManager.currentUser != null
                      ? AssetImage(SessionManager.currentUser!.avatar) : null,
                ),
              ],
            ),

            const SizedBox(height: 20),

            // --- SEARCH CỐ ĐỊNH ---
            Container(
              padding: const EdgeInsets.symmetric(horizontal: 16),
              decoration: BoxDecoration(color: Colors.grey[200], borderRadius: BorderRadius.circular(30)),
              child: TextField(
                controller: _searchController,
                decoration: const InputDecoration(
                  border: InputBorder.none,
                  icon: Icon(Icons.search, color: Colors.black54),
                  hintText: "Tìm kiếm",
                ),
              ),
            ),

            const SizedBox(height: 30),
            const Padding(padding: EdgeInsets.only(left: 20.0), child: Text("Danh sách bạn bè", style: TextStyle(fontSize: 23))),
            const SizedBox(height: 20),

            // --- VÙNG FRIEND AREA (KHUNG CUỘN) ---
            SizedBox(
              height: 3 * 80.0, // Cố định chiều cao hiển thị đúng 3 hàng (240px)
              child: ListView.builder(
                padding: EdgeInsets.zero,
                // Dùng AlwaysScrollableScrollPhysics để đảm bảo luôn cuộn được
                physics: const AlwaysScrollableScrollPhysics(),
                // Hiển thị toàn bộ danh sách, không giới hạn ở 3
                itemCount: _filteredList.length,
                itemBuilder: (context, index) {
                  final friend = _filteredList[index];
                  final String displayName = SessionManager.nicknameMap.containsKey(friend.friendId)
                      ? SessionManager.nicknameMap[friend.friendId]!
                      : friend.fullName;
                  return InkWell(
                    onTap: () {
                      SessionManager.selectedFriend = friend;
                      Navigator.push(context, MaterialPageRoute(builder: (context) => const ChatfriendScreen()));
                    },
                    child: Padding(
                      padding: const EdgeInsets.symmetric(vertical: 8, horizontal: 20),
                      child: Row(
                        children: [
                          Stack(
                            children: [
                              CircleAvatar(radius: 30, backgroundImage: AssetImage(friend.avatar)),
                              if (friend.isOnline)
                                Positioned(right: 0, bottom: 0, child: Container(width: 18, height: 18, decoration: const BoxDecoration(color: Colors.green, shape: BoxShape.circle))),
                            ],
                          ),
                          const SizedBox(width: 30),
                          Text(displayName, style: const TextStyle(fontSize: 23, fontWeight: FontWeight.w400)),
                        ],
                      ),
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }
}
