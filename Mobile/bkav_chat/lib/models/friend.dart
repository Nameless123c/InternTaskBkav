class Friend {
  String friendId;
  String fullName;
  String avatar;
  bool isOnline;

  Friend({
    required this.friendId,
    required this.fullName,
    this.avatar = 'avatar/default.jpg',
    required this.isOnline,
  });
}
