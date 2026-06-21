import 'dart:convert';
import 'package:http/http.dart' as http;

class ApiService {
  static Future<String> sendGetRequest(String url, {String? token}) {
    return http.get(
      Uri.parse(url),
      headers: {
        if (token != null && token.isNotEmpty) 'Authorization': 'Bearer $token',
      },
    )
        .timeout(const Duration(seconds: 15))
        .then((response) {
      return response.body;
    })
        .catchError((error) {
      return "";
    });
  }

  static Future<String> sendPostRequest(String url, Map<String, dynamic> jsonData, {String? token}) {
    return http.post(
      Uri.parse(url),
      headers: {
        'Content-Type': 'application/json',
        if (token != null && token.isNotEmpty) 'Authorization': 'Bearer $token',
      },
      body: jsonEncode(jsonData),
    )
        .timeout(const Duration(seconds: 15))
        .then((response) {
      return response.body;
    })
        .catchError((error) {
      return "";
    });
  }
}
