package com.oj.problem.exception;

import static org.junit.jupiter.api.Assertions.assertEquals;

import com.oj.problem.common.ApiResponse;
import org.junit.jupiter.api.Test;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.server.ResponseStatusException;

class GlobalExceptionHandlerTest {

    private final GlobalExceptionHandler handler = new GlobalExceptionHandler();

    @Test
    void handleBusinessExceptionShouldPreserveCodeAndStatus() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleBusinessException(
                new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));

        assertEquals(HttpStatus.NOT_FOUND, response.getStatusCode());
        assertEquals(404002, response.getBody().getCode());
    }

    @Test
    void handleResponseStatusExceptionShouldMapUnauthorizedCode() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleResponseStatusException(
                new ResponseStatusException(HttpStatus.UNAUTHORIZED, "未登录"));

        assertEquals(HttpStatus.UNAUTHORIZED, response.getStatusCode());
        assertEquals(401003, response.getBody().getCode());
    }

    @Test
    void handleIllegalArgumentExceptionShouldMapForbidden() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleIllegalArgumentException(
                new IllegalArgumentException("权限不足"));

        assertEquals(HttpStatus.FORBIDDEN, response.getStatusCode());
        assertEquals(403001, response.getBody().getCode());
    }

    @Test
    void handleUnexpectedExceptionShouldReturnInternalServerError() {
        ResponseEntity<ApiResponse<Void>> response = handler.handleUnexpectedException(new RuntimeException("boom"));

        assertEquals(HttpStatus.INTERNAL_SERVER_ERROR, response.getStatusCode());
        assertEquals(500001, response.getBody().getCode());
    }
}
