package com.oj.problem.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.exception.GlobalExceptionHandler;
import com.oj.problem.security.CurrentUser;
import com.oj.problem.security.JwtTokenService;
import com.oj.problem.service.ProblemService;
import java.time.LocalDateTime;
import java.util.Collections;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

@ExtendWith(MockitoExtension.class)
class ProblemControllerTest {

    @Mock
    private ProblemService problemService;

    @Mock
    private JwtTokenService jwtTokenService;

    private MockMvc mockMvc;
    private final ObjectMapper objectMapper = new ObjectMapper();

    @BeforeEach
    void setUp() {
        ProblemController controller = new ProblemController(problemService, jwtTokenService);
        mockMvc = MockMvcBuilders.standaloneSetup(controller)
                .setControllerAdvice(new GlobalExceptionHandler())
                .build();
    }

    @Test
    void listProblemsShouldReturnSuccessResponse() throws Exception {
        ProblemPageResponse response = new ProblemPageResponse();
        response.setTotal(1L);
        response.setPage(1);
        response.setSize(20);
        response.setProblems(Collections.emptyList());
        when(problemService.listProblems(any())).thenReturn(response);

        mockMvc.perform(get("/v1/problems?page=1&size=20"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.code").value(200))
                .andExpect(jsonPath("$.data.total").value(1))
                .andExpect(jsonPath("$.data.page").value(1));
    }

    @Test
    void getProblemShouldReturnProblemDetail() throws Exception {
        ProblemDetailResponse response = new ProblemDetailResponse();
        response.setId(1L);
        response.setTitle("两数之和");
        when(problemService.getProblemDetail(1L)).thenReturn(response);

        mockMvc.perform(get("/v1/problems/1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.data.id").value(1))
                .andExpect(jsonPath("$.data.title").value("两数之和"));
    }

    @Test
    void createProblemShouldReturnCreated() throws Exception {
        when(jwtTokenService.requireAdmin("Bearer token")).thenReturn(new CurrentUser(1L, "admin"));
        ProblemMutationResponse response = new ProblemMutationResponse();
        response.setId(2L);
        response.setTitle("新题目");
        response.setCreatedAt(LocalDateTime.now());
        when(problemService.createProblem(any(), any())).thenReturn(response);

        mockMvc.perform(post("/v1/problems")
                        .header("Authorization", "Bearer token")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(validRequestJson()))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.code").value(201))
                .andExpect(jsonPath("$.message").value("创建成功"))
                .andExpect(jsonPath("$.data.id").value(2));
    }

    @Test
    void updateProblemShouldReturnSuccessMessage() throws Exception {
        when(jwtTokenService.requireAdmin("Bearer token")).thenReturn(new CurrentUser(1L, "admin"));
        ProblemMutationResponse response = new ProblemMutationResponse();
        response.setId(3L);
        response.setTitle("更新题目");
        when(problemService.updateProblem(eq(3L), any(), any())).thenReturn(response);

        mockMvc.perform(put("/v1/problems/3")
                        .header("Authorization", "Bearer token")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(validRequestJson()))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.message").value("更新成功"))
                .andExpect(jsonPath("$.data.id").value(3));
    }

    @Test
    void deleteProblemShouldReturnSuccessMessage() throws Exception {
        when(jwtTokenService.requireAdmin("Bearer token")).thenReturn(new CurrentUser(1L, "admin"));
        doNothing().when(problemService).deleteProblem(eq(4L), any(CurrentUser.class));

        mockMvc.perform(delete("/v1/problems/4")
                        .header("Authorization", "Bearer token"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.message").value("删除成功"));
    }

    @Test
    void createProblemShouldReturnForbiddenWhenNoPermission() throws Exception {
        when(jwtTokenService.requireAdmin("Bearer no-admin")).thenThrow(new IllegalArgumentException("权限不足"));

        mockMvc.perform(post("/v1/problems")
                        .header("Authorization", "Bearer no-admin")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(validRequestJson()))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value(403001));
    }

    @Test
    void createProblemShouldReturnBadRequestWhenBodyInvalid() throws Exception {
        mockMvc.perform(post("/v1/problems")
                        .header("Authorization", "Bearer token")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"description\":\"only description\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value(400001));
    }

    private String validRequestJson() throws Exception {
        java.util.Map<String, Object> request = new java.util.LinkedHashMap<>();
        request.put("title", "题目");
        request.put("description", "描述");
        request.put("difficulty", "easy");
        request.put("timeLimit", 1000);
        request.put("memoryLimit", 128);
        request.put("inputDescription", "输入");
        request.put("outputDescription", "输出");
        request.put("sampleInput", "1 2");
        request.put("sampleOutput", "3");
        request.put("tags", java.util.Arrays.asList("数组", "哈希表"));
        request.put("isPublic", true);
        request.put("testCases", java.util.Arrays.asList(
                buildTestCase("1 2", "3", true, 20),
                buildTestCase("2 3", "5", false, 80)
        ));
        return objectMapper.writeValueAsString(request);
    }

    private java.util.Map<String, Object> buildTestCase(String input, String output, boolean isSample, int score) {
        java.util.Map<String, Object> testCase = new java.util.LinkedHashMap<>();
        testCase.put("input", input);
        testCase.put("output", output);
        testCase.put("isSample", isSample);
        testCase.put("score", score);
        return testCase;
    }
}
