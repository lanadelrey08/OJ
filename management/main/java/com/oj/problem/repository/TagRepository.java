package com.oj.problem.repository;

import com.oj.problem.entity.TagEntity;
import java.util.Collection;
import java.util.List;
import org.springframework.data.jpa.repository.JpaRepository;

public interface TagRepository extends JpaRepository<TagEntity, Long> {

    List<TagEntity> findByNameIn(Collection<String> names);
}
