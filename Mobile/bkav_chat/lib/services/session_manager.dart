import '../models/user.dart';
import '../models/friend.dart';

class SessionManager {
  static User? currentUser;
  static List<Friend> friendList = [];
  static String? token;
  static Friend? selectedFriend;
  static Map<String, String> nicknameMap = {};

  static void clearSession() {
    currentUser = null;
    friendList = [];
    token = null;
    selectedFriend = null;
    nicknameMap.clear();
  }
}