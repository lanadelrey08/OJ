package com.oj.problem.repository;

import com.oj.problem.entity.ProblemEntity;
import java.util.Optional;
import org.springframework.data.jpa.repository.EntityGraph;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

public interface ProblemRepository extends JpaRepository<ProblemEntity, Long>, JpaSpecificationExecutor<ProblemEntity> {

    @EntityGraph(attributePaths = {"testCases", "tags"})
    Optional<ProblemEntity> findWithTestCasesAndTagsById(Long id);
}
