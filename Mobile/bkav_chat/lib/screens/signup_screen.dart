import 'package:flutter/material.dart';
import 'package:bkav_chat/services/api_service.dart';
import 'dart:convert';

class SignupScreen extends StatefulWidget{
  const SignupScreen({super.key});

  @override
  State<SignupScreen> createState() => _SignupScreenState();
}

class _SignupScreenState extends State<SignupScreen> {
  final TextEditingController _fullNameController = TextEditingController();
  final TextEditingController _usernameController = TextEditingController();
  final TextEditingController _passwordController = TextEditingController();
  final TextEditingController _rePassController = TextEditingController();
  String _errorMessage = "";

  @override
  void dispose() {
    _fullNameController.dispose();
    _usernameController.dispose();
    _passwordController.dispose();
    _rePassController.dispose();
    super.dispose();
  }

  Future<void> _handleSignup() async {
    String fullName = _fullNameController.text;
    String username = _usernameController.text;
    String password = _passwordController.text;
    String rePass = _rePassController.text;

    if (fullName.isEmpty || username.isEmpty ||
        password.isEmpty || rePass.isEmpty) {

      setState(() {
        _errorMessage = "Thông tin không được để trống!";
      });
      return;
    }

    if (password != rePass){
      setState(() {
        _errorMessage = "Mật khẩu không khớp!";
      });
      return;
    }

    String url = "http://localhost:8888/api/auth/register";
    Map<String, dynamic> data = {
      "FullName": fullName,
      "Username": username,
      "Password": rePass,
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
      Navigator.pop(context);
    } else {
      String msg = jsonRes["message"];

      if (msg == "Username already exists"){
        setState(() {
          _errorMessage = "Tài khoản đã tồn tại!";
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
      appBar: AppBar(
        backgroundColor: Colors.transparent,
        elevation: 0,
        leading: IconButton(
          icon: const Icon(Icons.arrow_back, color: Colors.black),
          onPressed: () => Navigator.pop(context),
        ),
      ),
      // Bọc trong SingleChildScrollView để màn hình có thể cuộn khi bàn phím che mất
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const SizedBox(height: 16),

            const Center(
              child: Text(
                "Tạo tài khoản",
                style: TextStyle(
                  fontSize: 18,
                  color: Colors.black,
                ),
              ),
            ),

            const SizedBox(height: 16),

            // Tên hiển thị
            const Text("Tên hiển thị", style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500, color: Colors.black87)),
            TextField(
              controller: _fullNameController,
              decoration: const InputDecoration(contentPadding: EdgeInsets.symmetric(vertical: 4)),
            ),

            const SizedBox(height: 16),

            // Tài khoản
            const Text("Tài khoản", style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500, color: Colors.black87)),
            TextField(
              controller: _usernameController,
              decoration: const InputDecoration(contentPadding: EdgeInsets.symmetric(vertical: 4)),
            ),

            const SizedBox(height: 16),

            // Mật khẩu
            const Text("Mật khẩu", style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500, color: Colors.black87)),
            TextField(
              controller: _passwordController,
              obscureText: true,
              decoration: const InputDecoration(contentPadding: EdgeInsets.symmetric(vertical: 4)),
            ),

            const SizedBox(height: 16),

            // Nhập lại mật khẩu
            const Text("Nhập lại mật khẩu", style: TextStyle(fontSize: 16, fontWeight: FontWeight.w500, color: Colors.black87)),
            TextField(
              controller: _rePassController,
              obscureText: true,
              decoration: const InputDecoration(contentPadding: EdgeInsets.symmetric(vertical: 4)),
            ),

            const SizedBox(height: 40), // Thay thế khoảng trống lớn bằng con số vừa phải

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

            const SizedBox(height: 250), // Tạo khoảng cách trước nút bấm

            SizedBox(
              width: double.infinity,
              height: 50,
              child: ElevatedButton(
                onPressed: _handleSignup,
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFF6783E7),
                  shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
                ),
                child: const Text("Tạo tài khoản", style: TextStyle(color: Colors.white, fontSize: 16)),
              ),
            ),

            const SizedBox(height: 20), // Đệm dưới cùng để giao diện không sát mép
          ],
        ),
      ),
    );
  }
}