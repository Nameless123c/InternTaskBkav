class User {
  String fullName;
  String username;
  String avatar;

  User({
    required this.fullName,
    required this.username,
    this.avatar = 'avatar/default.jpg',
  });
}