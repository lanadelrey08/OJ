package com.oj.problem.service;

import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.security.CurrentUser;

public interface ProblemService {

    ProblemPageResponse listProblems(ProblemQueryRequest queryRequest);

    ProblemDetailResponse getProblemDetail(Long id);

    ProblemMutationResponse createProblem(ProblemUpsertRequest request, CurrentUser currentUser);

    ProblemMutationResponse updateProblem(Long id, ProblemUpsertRequest request, CurrentUser currentUser);

    void deleteProblem(Long id, CurrentUser currentUser);
}
