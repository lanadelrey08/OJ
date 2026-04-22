package com.oj.problem.controller;

import com.oj.problem.common.ApiResponse;
import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.security.CurrentUser;
import com.oj.problem.security.JwtTokenService;
import com.oj.problem.service.ProblemService;
import javax.validation.Valid;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/v1/problems")
public class ProblemController {

    private final ProblemService problemService;
    private final JwtTokenService jwtTokenService;

    public ProblemController(ProblemService problemService, JwtTokenService jwtTokenService) {
        this.problemService = problemService;
        this.jwtTokenService = jwtTokenService;
    }

    @GetMapping
    public ApiResponse<ProblemPageResponse> listProblems(@Valid ProblemQueryRequest queryRequest) {
        return ApiResponse.success(problemService.listProblems(queryRequest));
    }

    @GetMapping("/{id}")
    public ApiResponse<ProblemDetailResponse> getProblem(@PathVariable Long id) {
        return ApiResponse.success(problemService.getProblemDetail(id));
    }

    @PostMapping
    @ResponseStatus(HttpStatus.CREATED)
    public ApiResponse<ProblemMutationResponse> createProblem(
            @RequestHeader(HttpHeaders.AUTHORIZATION) String authorization,
            @Valid @RequestBody ProblemUpsertRequest request) {
        CurrentUser currentUser = requireAdmin(authorization);
        return ApiResponse.created("创建成功", problemService.createProblem(request, currentUser));
    }

    @PutMapping("/{id}")
    public ApiResponse<ProblemMutationResponse> updateProblem(
            @PathVariable Long id,
            @RequestHeader(HttpHeaders.AUTHORIZATION) String authorization,
            @Valid @RequestBody ProblemUpsertRequest request) {
        CurrentUser currentUser = requireAdmin(authorization);
        return ApiResponse.success("更新成功", problemService.updateProblem(id, request, currentUser));
    }

    @DeleteMapping("/{id}")
    public ApiResponse<Void> deleteProblem(
            @PathVariable Long id,
            @RequestHeader(HttpHeaders.AUTHORIZATION) String authorization) {
        CurrentUser currentUser = requireAdmin(authorization);
        problemService.deleteProblem(id, currentUser);
        return ApiResponse.success("删除成功", null);
    }

    private CurrentUser requireAdmin(String authorization) {
        return jwtTokenService.requireAdmin(authorization);
    }
}
