package com.oj.problem.service.impl;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.request.TestCaseRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.entity.Difficulty;
import com.oj.problem.entity.ProblemEntity;
import com.oj.problem.entity.TagEntity;
import com.oj.problem.entity.TestCaseEntity;
import com.oj.problem.exception.BusinessException;
import com.oj.problem.repository.ProblemRepository;
import com.oj.problem.repository.TagRepository;
import com.oj.problem.security.CurrentUser;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Optional;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.ArgumentCaptor;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.springframework.data.domain.PageImpl;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.domain.Specification;

@ExtendWith(MockitoExtension.class)
class ProblemServiceImplTest {

    @Mock
    private ProblemRepository problemRepository;

    @Mock
    private TagRepository tagRepository;

    @InjectMocks
    private ProblemServiceImpl problemService;

    private CurrentUser adminUser;

    @BeforeEach
    void setUp() {
        adminUser = new CurrentUser(100L, "admin");
    }

    @Test
    void listProblemsShouldMapPageResult() {
        ProblemQueryRequest queryRequest = new ProblemQueryRequest();
        queryRequest.setPage(2);
        queryRequest.setSize(5);

        ProblemEntity entity = buildProblemEntity(1L, "两数之和", true);
        entity.setSubmissionCount(20);
        entity.setAcceptedCount(15);
        entity.getTags().add(buildTag("数组", 3));

        when(problemRepository.findAll(any(Specification.class), any(Pageable.class)))
                .thenReturn(new PageImpl<>(Collections.singletonList(entity)));

        ProblemPageResponse response = problemService.listProblems(queryRequest);

        assertEquals(1L, response.getTotal());
        assertEquals(2, response.getPage());
        assertEquals(5, response.getSize());
        assertEquals(1, response.getProblems().size());
        assertEquals("两数之和", response.getProblems().get(0).getTitle());
        assertEquals(0.75D, response.getProblems().get(0).getPassRate());
    }

    @Test
    void getProblemDetailShouldReturnPublicProblem() {
        ProblemEntity entity = buildProblemEntity(2L, "最长子串", true);
        entity.setSubmissionCount(10);
        entity.setAcceptedCount(5);
        entity.getTags().add(buildTag("滑动窗口", 1));

        when(problemRepository.findWithTestCasesAndTagsById(2L)).thenReturn(Optional.of(entity));

        ProblemDetailResponse response = problemService.getProblemDetail(2L);

        assertEquals(2L, response.getId());
        assertEquals("最长子串", response.getTitle());
        assertEquals("medium", response.getDifficulty());
        assertEquals(0.5D, response.getPassRate());
        assertEquals(Collections.singletonList("滑动窗口"), response.getTags());
    }

    @Test
    void getProblemDetailShouldRejectPrivateProblem() {
        ProblemEntity entity = buildProblemEntity(3L, "私有题", false);
        when(problemRepository.findWithTestCasesAndTagsById(3L)).thenReturn(Optional.of(entity));

        BusinessException exception = assertThrows(BusinessException.class, () -> problemService.getProblemDetail(3L));

        assertEquals(404002, exception.getCode());
    }

    @Test
    void createProblemShouldPopulateEntityAndTags() {
        ProblemUpsertRequest request = buildUpsertRequest("新题目", Arrays.asList("数组", "哈希表"));
        when(tagRepository.findByNameIn(any())).thenReturn(Collections.emptyList());
        when(problemRepository.save(any(ProblemEntity.class))).thenAnswer(invocation -> {
            ProblemEntity saved = invocation.getArgument(0);
            saved.setId(9L);
            return saved;
        });

        ProblemMutationResponse response = problemService.createProblem(request, adminUser);

        ArgumentCaptor<ProblemEntity> captor = ArgumentCaptor.forClass(ProblemEntity.class);
        verify(problemRepository).save(captor.capture());
        ProblemEntity savedEntity = captor.getValue();

        assertEquals("新题目", savedEntity.getTitle());
        assertEquals(Long.valueOf(100L), savedEntity.getCreatedBy());
        assertEquals(2, savedEntity.getTags().size());
        assertEquals(2, savedEntity.getTestCases().size());
        assertTrue(savedEntity.getTags().stream().allMatch(tag -> tag.getProblemCount() == 1));
        assertEquals(9L, response.getId());
    }

    @Test
    void updateProblemShouldReplaceTagsAndTestCases() {
        ProblemEntity entity = buildProblemEntity(5L, "旧题目", true);
        TagEntity oldKeptTag = buildTag("数组", 2);
        TagEntity oldRemovedTag = buildTag("字符串", 1);
        entity.setTags(new LinkedHashSet<>(Arrays.asList(oldKeptTag, oldRemovedTag)));
        entity.addTestCase(buildTestCase("old", "old", false, 10));

        TagEntity existingKeptTag = buildTag("数组", 2);
        when(problemRepository.findWithTestCasesAndTagsById(5L)).thenReturn(Optional.of(entity));
        when(tagRepository.findByNameIn(any())).thenReturn(Collections.singletonList(existingKeptTag));
        when(problemRepository.save(any(ProblemEntity.class))).thenAnswer(invocation -> invocation.getArgument(0));

        ProblemUpsertRequest request = buildUpsertRequest("更新题目", Arrays.asList("数组", "图论"));
        ProblemMutationResponse response = problemService.updateProblem(5L, request, adminUser);

        assertEquals("更新题目", entity.getTitle());
        assertEquals(2, entity.getTags().size());
        assertTrue(entity.getTags().stream().anyMatch(tag -> "数组".equals(tag.getName())));
        assertTrue(entity.getTags().stream().anyMatch(tag -> "图论".equals(tag.getName()) && tag.getProblemCount() == 1));
        assertEquals(0, oldRemovedTag.getProblemCount().intValue());
        assertEquals(2, entity.getTestCases().size());
        assertEquals(5L, response.getId());
    }

    @Test
    void deleteProblemShouldDecreaseTagCountAndDeleteEntity() {
        ProblemEntity entity = buildProblemEntity(7L, "待删除题目", true);
        TagEntity tag = buildTag("数学", 1);
        entity.getTags().add(tag);

        when(problemRepository.findWithTestCasesAndTagsById(7L)).thenReturn(Optional.of(entity));

        problemService.deleteProblem(7L, adminUser);

        assertEquals(0, tag.getProblemCount().intValue());
        verify(problemRepository).delete(entity);
    }

    @Test
    void updateProblemShouldThrowWhenNotFound() {
        when(problemRepository.findWithTestCasesAndTagsById(99L)).thenReturn(Optional.empty());

        BusinessException exception = assertThrows(
                BusinessException.class,
                () -> problemService.updateProblem(99L, buildUpsertRequest("X", Collections.singletonList("数组")), adminUser));

        assertEquals(404002, exception.getCode());
    }

    private ProblemEntity buildProblemEntity(Long id, String title, boolean isPublic) {
        ProblemEntity entity = new ProblemEntity();
        entity.setId(id);
        entity.setTitle(title);
        entity.setDescription("题目描述");
        entity.setDifficulty(Difficulty.medium);
        entity.setTimeLimit(1000);
        entity.setMemoryLimit(128);
        entity.setInputDescription("输入说明");
        entity.setOutputDescription("输出说明");
        entity.setSampleInput("1 2");
        entity.setSampleOutput("3");
        entity.setIsPublic(isPublic);
        return entity;
    }

    private ProblemUpsertRequest buildUpsertRequest(String title, java.util.List<String> tags) {
        ProblemUpsertRequest request = new ProblemUpsertRequest();
        request.setTitle(title);
        request.setDescription("题目描述");
        request.setDifficulty(Difficulty.easy);
        request.setTimeLimit(1000);
        request.setMemoryLimit(128);
        request.setInputDescription("输入说明");
        request.setOutputDescription("输出说明");
        request.setSampleInput("1 2");
        request.setSampleOutput("3");
        request.setTags(tags);
        request.setTestCases(Arrays.asList(
                buildTestCaseRequest("1 2", "3", true, 20),
                buildTestCaseRequest("2 3", "5", false, 80)
        ));
        request.setIsPublic(true);
        return request;
    }

    private TestCaseRequest buildTestCaseRequest(String input, String output, boolean isSample, int score) {
        TestCaseRequest request = new TestCaseRequest();
        request.setInput(input);
        request.setOutput(output);
        request.setIsSample(isSample);
        request.setScore(score);
        return request;
    }

    private TestCaseEntity buildTestCase(String input, String output, boolean isSample, int score) {
        TestCaseEntity entity = new TestCaseEntity();
        entity.setInput(input);
        entity.setOutput(output);
        entity.setIsSample(isSample);
        entity.setScore(score);
        return entity;
    }

    private TagEntity buildTag(String name, int count) {
        TagEntity tag = new TagEntity();
        tag.setName(name);
        tag.setProblemCount(count);
        return tag;
    }
}
