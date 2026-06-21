class MediaItem {
  String id;
  String url;
  String fileName;

  MediaItem(this.id, this.url, this.fileName);
}

class Message {
  String id;
  String content;
  List<MediaItem> images;
  List<MediaItem> files;
  int isSend;
  String createdAt;
  int messageType;

  Message({
    required this.id,
    required this.content,
    required this.images,
    required this.files,
    required this.isSend,
    required this.createdAt,
    required this.messageType,
  });
}