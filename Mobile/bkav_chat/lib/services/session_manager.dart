import '../models/user.dart';
import '../models/friend.dart';

class SessionManager {
  static User? currentUser;
  static List<Friend> friendList = [];
  static String? token;
  static Friend? selectedFriend;

  static void clearSession() {
    currentUser = null;
    friendList = [];
    token = null;
    selectedFriend = null;
  }
}