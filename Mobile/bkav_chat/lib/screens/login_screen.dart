import 'package:flutter/material.dart';
import 'signup_screen.dart';
import 'homechat_screen.dart';
import '../services/api_service.dart';
import '../services/session_manager.dart';
import 'dart:convert';


class LoginScreen extends StatefulWidget {
  const LoginScreen({super.key});

  @override
  State<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends State<LoginScreen> {
  final TextEditingController _usernameController = TextEditingController();
  final TextEditingController _passwordController = TextEditingController();
  String _errorMessage = "";

  @override
  void dispose() {
    _usernameController.dispose();
    _passwordController.dispose();
    super.dispose();
  }

  Future<void> _handleLogin() async {
    String username = _usernameController.text;
    String password = _passwordController.text;

    if (username.isEmpty){
      setState(() {
        _errorMessage = "Tên đăng nhập không được để trống";
      });
      return;
    }

    if (password.isEmpty){
      setState(() {
        _errorMessage = "Mật khẩu không được để trống";
      });
      return;
    }

    String url = "http://localhost:8888/api/auth/login";
    Map<String, dynamic> data = {
      "Username": username,
      "Password": password,
    };
    String? token;

    String res = await ApiService.sendPostRequest(url, data, token: token);

    if (res.isEmpty){
      setState(() {
        _errorMessage = "Lỗi kết nối mạng!";
      });
      return;
    }

    var jsonRes = jsonDecode(res);

    if (jsonRes["status"] == 1) {
      SessionManager.token = jsonRes["data"]["token"];
      Navigator.push(context, MaterialPageRoute(builder: (context) => const HomechatScreen()));
    }
    else {
      String msg = jsonRes["message"];

      if (msg == "Username already exists" || msg == "Incorrect password"){
        setState(() {
          _errorMessage = "Bạn nhập sai tên tài khoản hoặc mật khẩu!";
        });
        return;
      }

      setState(() {
        _errorMessage = msg;
      });
      return;
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      // resizeToAvoidBottomInset: true giúp nội dung tự đẩy lên khi bàn phím xuất hiện
      resizeToAvoidBottomInset: true,
      body: SingleChildScrollView(
        child: Padding(
          padding: const EdgeInsets.all(20.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const SizedBox(height: 50), // Khoảng cách đầu trang

              const Text(
                "Bkav Chat",
                style: TextStyle(
                  fontSize: 28,
                  fontWeight: FontWeight.bold,
                  color: Color(0xFF6783E7),
                ),
              ),

              const SizedBox(height: 200),

              const Text("Tài khoản", style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500)),
              TextField(
                controller: _usernameController,
                decoration: const InputDecoration(contentPadding: EdgeInsets.symmetric(vertical: 4)),
              ),

              const SizedBox(height: 20),

              const Text("Mật khẩu", style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500)),
              TextField(
                controller: _passwordController,
                obscureText: true,
                decoration: const InputDecoration(contentPadding: EdgeInsets.symmetric(vertical: 4)),
              ),

              const SizedBox(height: 40),

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

              const SizedBox(height: 250),

              SizedBox(
                width: double.infinity,
                height: 50,
                child: ElevatedButton(
                  onPressed: _handleLogin,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: const Color(0xFF6783E7),
                    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
                  ),
                  child: const Text("Đăng nhập", style: TextStyle(color: Colors.white, fontSize: 16)),
                ),
              ),

              Center(
                child: TextButton(
                  onPressed: () => Navigator.push(
                      context,
                      MaterialPageRoute(builder: (context) => const SignupScreen())
                  ),
                  child: const Text("Đăng ký", style: TextStyle(fontSize: 16, color: Color(0xFF047DE7))),
                ),
              ),

              const SizedBox(height: 20), // Padding dưới cùng để tránh sát mép
            ],
          ),
        ),
      ),
    );
  }
}