package org.example.login;   // 请确保包名与你的启动类一致

import org.example.login.dao.UserDAO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;

@Controller
public class LoginController {

    @Autowired
    private  UserDAO userDAO;

    @GetMapping({"/", "index.html"})
    public String showIndexPage() {
        return "index";
    }

    @GetMapping("/login")
    public String showLoginPage() {
        return "login";
    }

    @GetMapping("/register")
    public String showRegisterPage() {
        return "register";
    }

    @PostMapping("/login")
    public String handleLogin(@RequestParam String username,
                              @RequestParam String password,
                              Model model) {
        // 从数据库查询该用户名对应的密码
        String PWD = userDAO.findPasswordByUsername(username);
        if (PWD.equals(password)) {
            model.addAttribute("username", username);
            return "/welcome";
        }
        else {
            model.addAttribute("error", "用户名或密码错误");
            return "/login";
        }
    }

    @PostMapping("/register")
    public String handleRegister(@RequestParam String username,
                                 @RequestParam String password,
                                 Model model) {
        // 在数据库中查询是否有人使用过该用户名
        boolean flag = userDAO.findUsername(username);
        if (flag) {
            model.addAttribute("error", "该用户名已被人使用");
            return "register";
        }
        else {
            int register = userDAO.registerUser(username, password);
            if (register > 0) {
                model.addAttribute("username", "注册用户成功");
                return "login";
            }
            else {
                model.addAttribute("error", "注册失败请联系服务器管理员");
                return "register";

            }
        }
    }
}