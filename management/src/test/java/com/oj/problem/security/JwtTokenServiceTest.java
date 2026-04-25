package com.oj.problem.security;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.fasterxml.jackson.databind.ObjectMapper;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

class JwtTokenServiceTest {

    private static final String SECRET = "unit-test-secret";

    private JwtTokenService jwtTokenService;
    private ObjectMapper objectMapper;

    @BeforeEach
    void setUp() {
        objectMapper = new ObjectMapper();
        jwtTokenService = new JwtTokenService(objectMapper, SECRET);
    }

    @Test
    void parseAuthorizationShouldReturnCurrentUser() throws Exception {
        String token = generateToken(1L, "admin");

        CurrentUser currentUser = jwtTokenService.parseAuthorization("Bearer " + token);

        assertEquals(1L, currentUser.getUserId());
        assertEquals("admin", currentUser.getRole());
    }

    @Test
    void requireAdminShouldRejectNonAdminRole() throws Exception {
        String token = generateToken(2L, "user");

        IllegalArgumentException exception = assertThrows(
                IllegalArgumentException.class,
                () -> jwtTokenService.requireAdmin("Bearer " + token));

        assertEquals("权限不足", exception.getMessage());
    }

    @Test
    void parseAuthorizationShouldRejectInvalidSignature() throws Exception {
        String token = generateToken(3L, "admin") + "broken";

        IllegalArgumentException exception = assertThrows(
                IllegalArgumentException.class,
                () -> jwtTokenService.parseAuthorization("Bearer " + token));

        assertEquals("Token无效", exception.getMessage());
    }

    @Test
    void parseAuthorizationShouldRejectMissingBearerPrefix() {
        IllegalArgumentException exception = assertThrows(
                IllegalArgumentException.class,
                () -> jwtTokenService.parseAuthorization("token"));

        assertEquals("未登录", exception.getMessage());
    }

    @Test
    void parseAuthorizationShouldSupportSubClaim() throws Exception {
        String token = generateTokenWithSub(8L, "admin");

        CurrentUser currentUser = jwtTokenService.parseAuthorization("Bearer " + token);

        assertEquals(8L, currentUser.getUserId());
    }

    private String generateToken(Long userId, String role) throws Exception {
        Map<String, Object> payload = new LinkedHashMap<>();
        payload.put("user_id", userId);
        payload.put("role", role);
        return signToken(payload);
    }

    private String generateTokenWithSub(Long userId, String role) throws Exception {
        Map<String, Object> payload = new LinkedHashMap<>();
        payload.put("sub", userId);
        payload.put("role", role);
        return signToken(payload);
    }

    private String signToken(Map<String, Object> payload) throws Exception {
        String headerJson = objectMapper.writeValueAsString(java.util.Collections.singletonMap("alg", "HS256"));
        String payloadJson = objectMapper.writeValueAsString(payload);
        String encodedHeader = Base64.getUrlEncoder().withoutPadding()
                .encodeToString(headerJson.getBytes(StandardCharsets.UTF_8));
        String encodedPayload = Base64.getUrlEncoder().withoutPadding()
                .encodeToString(payloadJson.getBytes(StandardCharsets.UTF_8));
        String signature = sign(encodedHeader + "." + encodedPayload);
        assertTrue(signature.length() > 0);
        return encodedHeader + "." + encodedPayload + "." + signature;
    }

    private String sign(String content) throws Exception {
        Mac mac = Mac.getInstance("HmacSHA256");
        SecretKeySpec keySpec = new SecretKeySpec(SECRET.getBytes(StandardCharsets.UTF_8), "HmacSHA256");
        mac.init(keySpec);
        return Base64.getUrlEncoder().withoutPadding()
                .encodeToString(mac.doFinal(content.getBytes(StandardCharsets.UTF_8)));
    }
}
