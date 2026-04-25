package com.oj.problem.service.impl;

import com.oj.problem.dto.request.ProblemQueryRequest;
import com.oj.problem.dto.request.ProblemUpsertRequest;
import com.oj.problem.dto.request.TestCaseRequest;
import com.oj.problem.dto.response.ProblemDetailResponse;
import com.oj.problem.dto.response.ProblemListItemResponse;
import com.oj.problem.dto.response.ProblemMutationResponse;
import com.oj.problem.dto.response.ProblemPageResponse;
import com.oj.problem.entity.ProblemEntity;
import com.oj.problem.entity.TagEntity;
import com.oj.problem.entity.TestCaseEntity;
import com.oj.problem.exception.BusinessException;
import com.oj.problem.repository.ProblemRepository;
import com.oj.problem.repository.TagRepository;
import com.oj.problem.security.CurrentUser;
import com.oj.problem.service.ProblemService;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import javax.persistence.criteria.Join;
import javax.persistence.criteria.JoinType;
import javax.persistence.criteria.Predicate;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.util.StringUtils;

@Service
public class ProblemServiceImpl implements ProblemService {

    private final ProblemRepository problemRepository;
    private final TagRepository tagRepository;

    public ProblemServiceImpl(ProblemRepository problemRepository, TagRepository tagRepository) {
        this.problemRepository = problemRepository;
        this.tagRepository = tagRepository;
    }

    @Override
    @Transactional(readOnly = true)
    public ProblemPageResponse listProblems(ProblemQueryRequest queryRequest) {
        Pageable pageable = PageRequest.of(
                queryRequest.getPage() - 1,
                queryRequest.getSize(),
                Sort.by(Sort.Direction.DESC, "createdAt"));

        Page<ProblemEntity> page = problemRepository.findAll(buildSpecification(queryRequest), pageable);
        ProblemPageResponse response = new ProblemPageResponse();
        response.setTotal(page.getTotalElements());
        response.setPage(queryRequest.getPage());
        response.setSize(queryRequest.getSize());
        response.setProblems(page.getContent().stream().map(this::toListItem).collect(Collectors.toList()));
        return response;
    }

    @Override
    @Transactional(readOnly = true)
    public ProblemDetailResponse getProblemDetail(Long id) {
        ProblemEntity problem = problemRepository.findWithTestCasesAndTagsById(id)
                .filter(ProblemEntity::getIsPublic)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        return toDetail(problem);
    }

    @Override
    @Transactional
    public ProblemMutationResponse createProblem(ProblemUpsertRequest request, CurrentUser currentUser) {
        ProblemEntity entity = new ProblemEntity();
        applyRequest(entity, request, currentUser);
        ProblemEntity saved = problemRepository.save(entity);
        return toMutation(saved);
    }

    @Override
    @Transactional
    public ProblemMutationResponse updateProblem(Long id, ProblemUpsertRequest request, CurrentUser currentUser) {
        ProblemEntity entity = problemRepository.findWithTestCasesAndTagsById(id)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        applyRequest(entity, request, currentUser);
        ProblemEntity saved = problemRepository.save(entity);
        return toMutation(saved);
    }

    @Override
    @Transactional
    public void deleteProblem(Long id, CurrentUser currentUser) {
        ProblemEntity entity = problemRepository.findWithTestCasesAndTagsById(id)
                .orElseThrow(() -> new BusinessException(404002, "题目不存在", HttpStatus.NOT_FOUND));
        decreaseTagCount(entity.getTags());
        problemRepository.delete(entity);
    }

    private Specification<ProblemEntity> buildSpecification(ProblemQueryRequest queryRequest) {
        return (root, query, cb) -> {
            List<Predicate> predicates = new ArrayList<>();
            predicates.add(cb.isTrue(root.get("isPublic")));

            if (queryRequest.getDifficulty() != null) {
                predicates.add(cb.equal(root.get("difficulty"), queryRequest.getDifficulty()));
            }

            if (StringUtils.hasText(queryRequest.getKeyword())) {
                String keyword = "%" + queryRequest.getKeyword().trim().toLowerCase(Locale.ROOT) + "%";
                predicates.add(cb.or(
                        cb.like(cb.lower(root.get("title")), keyword),
                        cb.like(cb.lower(root.get("description")), keyword)
                ));
            }

            if (StringUtils.hasText(queryRequest.getTag())) {
                Join<Object, Object> tagJoin = root.join("tags", JoinType.LEFT);
                predicates.add(cb.equal(cb.lower(tagJoin.get("name")), queryRequest.getTag().trim().toLowerCase(Locale.ROOT)));
                query.distinct(true);
            }

            return cb.and(predicates.toArray(new Predicate[0]));
        };
    }

    private void applyRequest(ProblemEntity entity, ProblemUpsertRequest request, CurrentUser currentUser) {
        entity.setTitle(request.getTitle().trim());
        entity.setDescription(request.getDescription().trim());
        entity.setDifficulty(request.getDifficulty());
        entity.setTimeLimit(request.getTimeLimit());
        entity.setMemoryLimit(request.getMemoryLimit());
        entity.setInputDescription(request.getInputDescription().trim());
        entity.setOutputDescription(request.getOutputDescription().trim());
        entity.setSampleInput(request.getSampleInput());
        entity.setSampleOutput(request.getSampleOutput());
        entity.setIsPublic(Boolean.TRUE.equals(request.getIsPublic()));
        if (entity.getCreatedBy() == null) {
            entity.setCreatedBy(currentUser.getUserId());
        }

        syncTestCases(entity, request.getTestCases());
        syncTags(entity, request.getTags());
    }

    private void syncTestCases(ProblemEntity entity, List<TestCaseRequest> testCases) {
        entity.clearTestCases();
        for (TestCaseRequest request : testCases) {
            TestCaseEntity testCase = new TestCaseEntity();
            testCase.setInput(request.getInput());
            testCase.setOutput(request.getOutput());
            testCase.setIsSample(request.getIsSample());
            testCase.setScore(request.getScore());
            entity.addTestCase(testCase);
        }
    }

    private void syncTags(ProblemEntity entity, List<String> requestedTags) {
        List<String> normalizedNames = requestedTags == null
                ? Collections.emptyList()
                : requestedTags.stream()
                        .filter(StringUtils::hasText)
                        .map(String::trim)
                        .distinct()
                        .collect(Collectors.toList());

        Set<TagEntity> oldTags = new LinkedHashSet<>(entity.getTags());
        List<TagEntity> existingTags = normalizedNames.isEmpty()
                ? Collections.emptyList()
                : tagRepository.findByNameIn(normalizedNames);
        Map<String, TagEntity> existingMap = existingTags.stream()
                .collect(Collectors.toMap(TagEntity::getName, Function.identity()));

        Set<TagEntity> newTags = new LinkedHashSet<>();
        Set<String> newTagNames = new LinkedHashSet<>();
        for (String name : normalizedNames) {
            TagEntity tag = existingMap.get(name);
            if (tag == null) {
                tag = new TagEntity();
                tag.setName(name);
                tag.setProblemCount(0);
            }
            newTags.add(tag);
            newTagNames.add(name);
        }

        Set<String> oldTagNames = oldTags.stream().map(TagEntity::getName).collect(Collectors.toSet());
        Set<TagEntity> tagsToAdd = newTags.stream()
                .filter(tag -> !oldTagNames.contains(tag.getName()))
                .collect(Collectors.toCollection(LinkedHashSet::new));
        Set<TagEntity> tagsToRemove = oldTags.stream()
                .filter(tag -> !newTagNames.contains(tag.getName()))
                .collect(Collectors.toCollection(LinkedHashSet::new));

        for (TagEntity tag : tagsToAdd) {
            tag.setProblemCount(tag.getProblemCount() + 1);
        }

        decreaseTagCount(tagsToRemove);

        entity.getTags().clear();
        entity.getTags().addAll(newTags);
    }

    private void decreaseTagCount(Set<TagEntity> tags) {
        for (TagEntity tag : tags) {
            int current = tag.getProblemCount() == null ? 0 : tag.getProblemCount();
            tag.setProblemCount(Math.max(0, current - 1));
        }
    }

    private ProblemListItemResponse toListItem(ProblemEntity entity) {
        ProblemListItemResponse response = new ProblemListItemResponse();
        response.setId(entity.getId());
        response.setTitle(entity.getTitle());
        response.setDifficulty(entity.getDifficulty().name());
        response.setTags(entity.getTags().stream().map(TagEntity::getName).collect(Collectors.toList()));
        response.setSubmissionCount(entity.getSubmissionCount());
        response.setAcceptedCount(entity.getAcceptedCount());
        response.setPassRate(calculatePassRate(entity.getSubmissionCount(), entity.getAcceptedCount()));
        return response;
    }

    private ProblemDetailResponse toDetail(ProblemEntity entity) {
        ProblemDetailResponse response = new ProblemDetailResponse();
        response.setId(entity.getId());
        response.setTitle(entity.getTitle());
        response.setDescription(entity.getDescription());
        response.setDifficulty(entity.getDifficulty().name());
        response.setTimeLimit(entity.getTimeLimit());
        response.setMemoryLimit(entity.getMemoryLimit());
        response.setInputDescription(entity.getInputDescription());
        response.setOutputDescription(entity.getOutputDescription());
        response.setSampleInput(entity.getSampleInput());
        response.setSampleOutput(entity.getSampleOutput());
        response.setTags(entity.getTags().stream().map(TagEntity::getName).collect(Collectors.toList()));
        response.setSubmissionCount(entity.getSubmissionCount());
        response.setAcceptedCount(entity.getAcceptedCount());
        response.setPassRate(calculatePassRate(entity.getSubmissionCount(), entity.getAcceptedCount()));
        response.setIsPublic(entity.getIsPublic());
        response.setCreatedAt(entity.getCreatedAt());
        response.setUpdatedAt(entity.getUpdatedAt());
        return response;
    }

    private ProblemMutationResponse toMutation(ProblemEntity entity) {
        ProblemMutationResponse response = new ProblemMutationResponse();
        response.setId(entity.getId());
        response.setTitle(entity.getTitle());
        response.setCreatedAt(entity.getCreatedAt());
        response.setUpdatedAt(entity.getUpdatedAt());
        return response;
    }

    private Double calculatePassRate(Integer submissionCount, Integer acceptedCount) {
        if (submissionCount == null || submissionCount == 0) {
            return 0D;
        }
        return acceptedCount == null ? 0D : acceptedCount.doubleValue() / submissionCount.doubleValue();
    }
}
